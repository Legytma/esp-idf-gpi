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
 * gpi_event.h
 *
 *  Created on: 23 de abr de 2021
 *      Author: Alex Manoel Ferreira Silva
 */

#pragma once

#include "esp_event_base.h"

// Declare an event base
ESP_EVENT_DECLARE_BASE(GPI_EVENT);

enum {                // declaration of the specific events
	GPI_EVENT_CHANGE, // raised when value change
	GPI_EVENT_WRITE,  // raised when value change
};

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
