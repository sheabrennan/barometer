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
#include "mgos_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

enum mgos_barometer_type {
  BARO_NONE = 0,
  BARO_MPL115,
  BARO_MPL3115,
  BARO_BME280,    // Also BMP280
  BARO_MS5611
};

struct mgos_barometer;

struct mgos_barometer_stats {
  double   last_read_time;       // value of mg_time() upon last call to _read()
  uint32_t read;                 // calls to _read()
  uint32_t read_success;         // successful _read()
  uint32_t read_success_cached;  // calls to _read() which were cached
  // Note: read_errors := read - read_success - read_success_cached
  double   read_success_usecs;   // time spent in successful uncached _read()
};

struct mgos_barometer *mgos_barometer_create_i2c(struct mgos_i2c *i2c, uint8_t i2caddr, enum mgos_barometer_type type);
void mgos_barometer_destroy(struct mgos_barometer **sensor);

bool mgos_barometer_has_thermometer(struct mgos_barometer *sensor);
bool mgos_barometer_has_barometer(struct mgos_barometer *sensor);
bool mgos_barometer_has_hygrometer(struct mgos_barometer *sensor);

/* Set cache TTL -- will limit reads and return cached data. Set msecs=0 to turn off */
bool mgos_barometer_set_cache_ttl(struct mgos_barometer *sensor, uint16_t msecs);

/* Read all available sensor data from the barometer */
bool mgos_barometer_read(struct mgos_barometer *sensor);

/* Return barometer data in units of Pascals */
bool mgos_barometer_get_pressure(struct mgos_barometer *sensor, float *p);

/* Return temperature data in units of Celsius */
bool mgos_barometer_get_temperature(struct mgos_barometer *sensor, float *t);

/* Return humidity data in units of % Relative Humidity */
bool mgos_barometer_get_humidity(struct mgos_barometer *sensor, float *h);

/* String representation of the barometer type, guaranteed to be 10 characters or less. */
const char *mgos_barometer_get_name(struct mgos_barometer *sensor);

/*
 * Return statistics on the sensor.
 */
bool mgos_barometer_get_stats(struct mgos_barometer *sensor, struct mgos_barometer_stats *stats);


/*
 * Initialization function for MGOS -- currently a noop.
 */
bool mgos_barometer_init(void);

#ifdef __cplusplus
}
#endif
