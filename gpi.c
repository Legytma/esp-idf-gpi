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

/*
 * gpi_ar.c
 *
 *  Created on: 22 de abr de 2021
 *      Author: Alex Manoel Ferreira Silva
 */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/gpio_struct.h"
#include "hal/gpio_types.h"
#include "hal/gpio_ll.h"
#include "driver/gpio.h"

#include "esp_event.h"
#include "esp_err.h"
#include "esp_bit_defs.h"
#include "esp_wifi_types.h"

#include "log_utils.h"

#include "gpi.h"
#include "gpi_event.h"

#include "sdkconfig.h"

LOG_TAG("gpi");

/* Event source definitions */
ESP_EVENT_DEFINE_BASE(GPI_EVENT);

static TaskHandle_t gpi_task_handle = NULL;

static void gpi_write(uint64_t output_mask, uint64_t value) {
	GPIO.out = (GPIO.out & ((uint32_t)~output_mask)) |
			   (((uint32_t)value) & ((uint32_t)output_mask));
	GPIO.out1.data =
		(GPIO.out1.data & ((uint32_t)(~output_mask >> 32))) |
		(((uint32_t)(value >> 32)) & ((uint32_t)(output_mask >> 32)));
}

static void gpi_write_event_handler(void* handler_args, esp_event_base_t base,
									int32_t id, void* event_data) {
	LOGD("gpi_write_event_handler(%s:GPI_EVENT_WRITE)", base);

	gpi_config_t* gpi_config = (gpi_config_t*)handler_args;

	gpi_config->output_value = *((uint64_t*)event_data);

	if (gpi_config->output_config.pin_bit_mask > 0) {
		gpi_write(gpi_config->output_config.pin_bit_mask, gpi_config->output_value);
	}
}

static uint64_t gpi_get(uint64_t input_mask) {
	return (GPIO.in | ((uint64_t)GPIO.in1.data << 32)) & input_mask;
}

static void gpi_task(void* arg) {
	LOG_FUN_START_D;

	gpi_config_t* gpi_config = (gpi_config_t*)arg;
	uint64_t      input_mask = gpi_config->input_config.pin_bit_mask;
	TickType_t    timeout    = pdMS_TO_TICKS(gpi_config->threshold);

	if (timeout == 0) {
		timeout = 1;
	}

	TickType_t current_time  = xTaskGetTickCount();
	uint64_t   current_value = 0;
	uint32_t   notify        = ulTaskNotifyTake(pdTRUE, 0);

	while (notify == 0) {
		uint64_t new_value = gpi_get(input_mask);

		for (int i = 0; (notify == 0) && (i < gpi_config->sample_count); i++) {
			vTaskDelayUntil(&current_time, timeout);

			new_value &= gpi_get(input_mask);

			current_time = xTaskGetTickCount();
			notify |= ulTaskNotifyTake(pdTRUE, 0);
		}

		if ((notify == 0) && ((new_value ^ current_value) != 0)) {
			current_value = new_value;

			ESP_ERROR_CHECK(esp_event_post(GPI_EVENT, GPI_EVENT_CHANGE,
										   &current_value, sizeof(uint64_t),
										   portMAX_DELAY));
		}
	}

	gpi_task_handle = NULL;

	LOG_FUN_END_D;

	vTaskDelete(NULL);
}

esp_err_t gpi_init(gpi_config_t* gpi_config) {
	LOG_FUN_START_D;

	if (gpi_task_handle != NULL) {
		return ESP_ERR_INVALID_STATE;
	}

	if (gpi_config->sample_count < 1) {
		return ESP_ERR_INVALID_ARG;
	}

	if (gpi_config->input_config.pin_bit_mask > 0) {
		// configure GPIO with the given settings
		esp_err_t ret = gpio_config(&gpi_config->input_config);

		if (ret != ESP_OK) {
			return ret;
		}
	}

	if (gpi_config->output_config.pin_bit_mask > 0) {

		esp_err_t ret = gpio_config(&gpi_config->output_config);

		if (ret != ESP_OK) {
			return ret;
		}
	}

	gpi_write(gpi_config->output_config.pin_bit_mask, gpi_config->output_value);

	// start gpio task
	esp_err_t ret = pdPASS != xTaskCreate(gpi_task, "gpi_task", 2048,
										  gpi_config, 10, &gpi_task_handle);

	if (ret != ESP_OK) {
		return ret;
	}

	// Register the specific event handlers.
	ret = esp_event_handler_register(GPI_EVENT, GPI_EVENT_WRITE,
									 gpi_write_event_handler, gpi_config);

	if (ret != ESP_OK) {
		return ret;
	}

	// printf("Minimum free heap size: %d bytes\n",
	// 	   esp_get_minimum_free_heap_size());

	return ESP_OK;
}

bool gpi_is_initialized() {
	return gpi_task_handle != NULL;
}

esp_err_t gpi_set(uint64_t value) {
	return esp_event_post(GPI_EVENT, GPI_EVENT_WRITE, &value, sizeof(uint64_t),
						  portMAX_DELAY);
}

esp_err_t gpi_deinit() {
	LOG_FUN_START_V;

	if (gpi_task_handle == NULL) {
		return ESP_ERR_INVALID_STATE;
	}

	xTaskNotifyGive(gpi_task_handle);

	return esp_event_handler_unregister(GPI_EVENT, GPI_EVENT_WRITE,
										gpi_write_event_handler);
}
