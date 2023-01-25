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
 * gpi.h
 *
 *  Created on: 22 de abr de 2021
 *      Author: Alex Manoel Ferreira Silva
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "esp_err.h"

typedef struct gpi_config_s {
	gpio_config_t input_config;
	gpio_config_t output_config;
	uint32_t threshold;
	size_t   sample_count;
	uint64_t output_value;
} gpi_config_t;

#ifdef __cplusplus
extern "C" {
#endif
esp_err_t gpi_init(gpi_config_t* gpi_config);
bool      gpi_is_initialized();
esp_err_t gpi_set(uint64_t value);
esp_err_t gpi_deinit();
#ifdef __cplusplus
}
#endif
