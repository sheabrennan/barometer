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
#include "mgos_barometer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Barometer
typedef bool (*mgos_barometer_mag_detect_fn)(struct mgos_barometer *dev);
typedef bool (*mgos_barometer_mag_create_fn)(struct mgos_barometer *dev);
typedef bool (*mgos_barometer_mag_destroy_fn)(struct mgos_barometer *dev);
typedef bool (*mgos_barometer_mag_read_fn)(struct mgos_barometer *dev);

#define MGOS_BAROMETER_CAP_BAROMETER      (0x01)
#define MGOS_BAROMETER_CAP_THERMOMETER    (0x02)
#define MGOS_BAROMETER_CAP_HYGROMETER     (0x04)

struct mgos_barometer {
  struct mgos_i2c *             i2c;
  uint8_t                       i2caddr;
  uint16_t                      cache_ttl_ms;
  enum mgos_barometer_type      type;

  uint8_t                       capabilities;

  mgos_barometer_mag_detect_fn  detect;
  mgos_barometer_mag_create_fn  create;
  mgos_barometer_mag_destroy_fn destroy;
  mgos_barometer_mag_read_fn    read;

  void *                        user_data;

  float                         pressure;    // in Pascals
  float                         temperature; // in Celcius
  float                         humidity;    // in % Relative Humidity

  struct mgos_barometer_stats   stats;
};

#ifdef __cplusplus
}
#endif
