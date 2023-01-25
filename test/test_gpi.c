/**
 * Copyright 2023 Legytma Soluções Inteligentes LTDA
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* test_mean.c: Implementation of a testable component.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#ifdef CONFIG_ESP_TASK_WDT
	#include "esp_task_wdt.h"
#endif
#include "esp_bit_defs.h"
#include "esp_event.h"
#include "unity.h"
#include "unity_test_runner.h"

#include "log_utils.h"
#include "gpi.h"
#include "gpi_event.h"

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif
static inline void test_gpi_init();
static inline void test_gpi_deinit();
#ifdef __cplusplus
}
#endif

LOG_TAG("test_gpi");

#define INPUT1 GPIO_NUM_36
#define INPUT2 GPIO_NUM_39
#define INPUT3 GPIO_NUM_34
#define INPUT4 GPIO_NUM_35

static const char* get_id_string(esp_event_base_t base, int32_t id) {
	if (base == GPI_EVENT) {
		switch (id) {
			case GPI_EVENT_CHANGE:
				return "GPI_EVENT_CHANGE";
		}
	} else {
		return "TASK_ITERATION_EVENT";
	}

	return "";
}

static void remove_idle_from_watchdog() {
#ifdef CONFIG_ESP_TASK_WDT
	#ifdef CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU0
	TaskHandle_t idle_0 = xTaskGetIdleTaskHandleForCPU(0);

	if (idle_0 != NULL) {
		esp_task_wdt_delete(idle_0);
	}
	#endif

	#ifdef CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU1
	TaskHandle_t idle_1 = xTaskGetIdleTaskHandleForCPU(1);

	if (idle_1 != NULL) {
		esp_task_wdt_delete(idle_1);
	}
	#endif
#endif
}

// Test declarations
TEST_CASE("Test gpi_init", "[test_gpi]") {
	test_gpi_init();
}

// Test declarations
TEST_CASE("Test gpi_deinit", "[test_gpi]") {
	test_gpi_deinit();
}

static gpi_config_t gpi_config = {
	.input_mask = BIT64(INPUT1) | BIT64(INPUT2) | BIT64(INPUT3) | BIT64(INPUT4),
	.threshold  = 10,
	.sample_count = 3,
};

static void gpi_write_event_handler(void* handler_args, esp_event_base_t base,
									int32_t id, void* event_data) {
	LOGI("gpi_write_event_handler(%s:%s)", base, get_id_string(base, id));

	uint64_t event_value = *((uint64_t*)event_data);

	LOGI(U64_STR "  %08X%08X  %llu", PRINT_U64(event_value),
		 (uint32_t)(event_value >> 32), (uint32_t)event_value, event_value);
}

// Test methods
static inline void test_gpi_init() {
	remove_idle_from_watchdog();

	// Create the default event loop
	esp_event_loop_create_default();

	// Register the specific timer event handlers.
	ESP_ERROR_CHECK(esp_event_handler_register(
		GPI_EVENT, GPI_EVENT_CHANGE, gpi_write_event_handler, &gpi_config));

	TEST_ASSERT_EQUAL_INT(ESP_OK, gpi_init(&gpi_config));

	LOGI("gpi initialized!");
}

// Test methods
static inline void test_gpi_deinit() {
	remove_idle_from_watchdog();

	TEST_ASSERT_EQUAL_INT(ESP_OK, gpi_deinit(&gpi_config));

	LOGI("gpi finalized!");
}
