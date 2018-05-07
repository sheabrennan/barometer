/*
 * Copyright 2018 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "mgos.h"
#include "mgos_barometer_internal.h"

#define MPL115_REG_PRESSURE         (0x00)
#define MPL115_REG_TEMPERATURE             (0x02)
#define MPL115_REG_COEFF_BASE         (0x04)
#define MPL115_REG_START      (0x12)

struct mgos_barometer_mpl115_data {
  float a0, b1, b2, c12;
};

bool mgos_barometer_mpl115_create(struct mgos_barometer *dev);
bool mgos_barometer_mpl115_destroy(struct mgos_barometer *dev);
bool mgos_barometer_mpl115_read(struct mgos_barometer *dev);
