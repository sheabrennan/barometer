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

#include "mgos.h"
#include "mgos_barometer_internal.h"
#include "mgos_barometer_mpl115.h"
#include "mgos_barometer_mpl3115.h"
#include "mgos_barometer_bme280.h"
#include "mgos_barometer_ms5611.h"

// Private functions follow
// Private functions end

// Public functions follow
struct mgos_barometer *mgos_barometer_create_i2c(struct mgos_i2c *i2c, uint8_t i2caddr, enum mgos_barometer_type type) {
  struct mgos_barometer *sensor;

  if (!i2c) {
    return NULL;
  }

  sensor = calloc(1, sizeof(struct mgos_barometer));
  if (!sensor) {
    return NULL;
  }
  memset(sensor, 0, sizeof(struct mgos_barometer));
  sensor->i2c     = i2c;
  sensor->i2caddr = i2caddr;
  switch (type) {
  case BARO_MPL115:
    sensor->create  = mgos_barometer_mpl115_create;
    sensor->read    = mgos_barometer_mpl115_read;
    sensor->destroy = mgos_barometer_mpl115_destroy;
    break;

  case BARO_MPL3115:
    sensor->detect = mgos_barometer_mpl3115_detect;
    sensor->create = mgos_barometer_mpl3115_create;
    sensor->read   = mgos_barometer_mpl3115_read;
    break;

  case BARO_BME280:
    sensor->detect  = mgos_barometer_bme280_detect;
    sensor->create  = mgos_barometer_bme280_create;
    sensor->read    = mgos_barometer_bme280_read;
    sensor->destroy = mgos_barometer_bme280_destroy;
    break;

  case BARO_MS5611:
    sensor->create  = mgos_barometer_ms5611_create;
    sensor->read    = mgos_barometer_ms5611_read;
    sensor->destroy = mgos_barometer_ms5611_destroy;
    break;

  default:
    LOG(LL_ERROR, ("Unknown mgos_barometer_type %d", type));
    free(sensor);
    return NULL;
  }
  sensor->type = type;
  if (sensor->detect) {
    if (!sensor->detect(sensor)) {
      LOG(LL_ERROR, ("Could not detect mgos_barometer_type %d at I2C 0x%02x", type, i2caddr));
      free(sensor);
      return NULL;
    } else {
      LOG(LL_DEBUG, ("Successfully detected mgos_barometer_type %d at I2C 0x%02x", type, i2caddr));
    }
  }

  if (sensor->create) {
    if (!sensor->create(sensor)) {
      LOG(LL_ERROR, ("Could not create mgos_barometer_type %d at I2C 0x%02x", type, i2caddr));
      free(sensor);
      return NULL;
    } else {
      LOG(LL_DEBUG, ("Successfully created mgos_barometer_type %d at I2C 0x%02x", type, i2caddr));
    }
  }

  return sensor;
}

void mgos_barometer_destroy(struct mgos_barometer **sensor) {
  if (!*sensor) {
    return;
  }
  if ((*sensor)->destroy && !(*sensor)->destroy(*sensor)) {
    LOG(LL_ERROR, ("Could not destroy mgos_barometer_type %d at I2C 0x%02x", (*sensor)->type, (*sensor)->i2caddr));
  }
  if ((*sensor)->user_data) {
    free((*sensor)->user_data);
  }
  free(*sensor);
  *sensor = NULL;
  return;
}

bool mgos_barometer_has_thermometer(struct mgos_barometer *sensor) {
  if (!sensor) {
    return false;
  }
  return sensor->capabilities & MGOS_BAROMETER_CAP_THERMOMETER;
}

bool mgos_barometer_has_barometer(struct mgos_barometer *sensor) {
  if (!sensor) {
    return false;
  }
  return sensor->capabilities & MGOS_BAROMETER_CAP_BAROMETER;
}

bool mgos_barometer_has_hygrometer(struct mgos_barometer *sensor) {
  if (!sensor) {
    return false;
  }
  return sensor->capabilities & MGOS_BAROMETER_CAP_HYGROMETER;
}

bool mgos_barometer_read(struct mgos_barometer *sensor) {
  double start = mg_time();
  bool   ret   = false;

  if (!sensor) {
    return false;
  }
  if (!sensor->read) {
    return false;
  }

  sensor->stats.read++;
  if (1000 * (start - sensor->stats.last_read_time) < sensor->cache_ttl_ms) {
    sensor->stats.read_success_cached++;
    return true;
  }

  ret = sensor->read(sensor);
  if (ret) {
    sensor->stats.read_success++;
    sensor->stats.read_success_usecs += 1000000 * (mg_time() - start);
    sensor->stats.last_read_time      = start;
  }
  return ret;
}

bool mgos_barometer_get_pressure(struct mgos_barometer *sensor, float *p) {
  if (!mgos_barometer_has_barometer(sensor)) {
    return false;
  }
  if (!mgos_barometer_read(sensor)) {
    return false;
  }
  if (*p) {
    *p = sensor->pressure;
  }
  return true;
}

bool mgos_barometer_get_temperature(struct mgos_barometer *sensor, float *t) {
  if (!mgos_barometer_has_thermometer(sensor)) {
    return false;
  }
  if (!mgos_barometer_read(sensor)) {
    return false;
  }
  if (*t) {
    *t = sensor->temperature;
  }
  return true;
}

bool mgos_barometer_get_humidity(struct mgos_barometer *sensor, float *h) {
  if (!mgos_barometer_has_hygrometer(sensor)) {
    return false;
  }
  if (!mgos_barometer_read(sensor)) {
    return false;
  }
  if (*h) {
    *h = sensor->humidity;
  }
  return true;
}

bool mgos_barometer_set_cache_ttl(struct mgos_barometer *sensor, uint16_t msecs) {
  if (!sensor) {
    return false;
  }
  sensor->cache_ttl_ms = msecs;
  return true;
}

bool mgos_barometer_get_stats(struct mgos_barometer *sensor, struct mgos_barometer_stats *stats) {
  if (!sensor || !stats) {
    return false;
  }

  memcpy((void *)stats, (const void *)&sensor->stats, sizeof(struct mgos_barometer_stats));
  return true;
}

const char *mgos_barometer_get_name(struct mgos_barometer *sensor) {
  if (!sensor) {
    return "Unknown";
  }
  switch (sensor->type) {
  case BARO_MPL115: return "MPL115";

  case BARO_MPL3115: return "MPL3115";

  case BARO_BME280:
    if (mgos_barometer_has_hygrometer(sensor)) {
      return "BME280";
    } else{
      return "BMP280";
    }

  case BARO_MS5611: return "MS5611";

  default: return "UNKNOWN";
  }
}

int mgos_barometer_return_capabilities(struct mgos_barometer *sensor){
  return sensor->capabilities;
}

float mgos_barometer_return_spec(struct mgos_barometer *sensor, uint8_t cap){
  if(cap & MGOS_BAROMETER_CAP_HYGROMETER){
    return sensor->humidity;
  }
  if(cap & MGOS_BAROMETER_CAP_THERMOMETER){
    return sensor->temperature;
  }
  if(cap & MGOS_BAROMETER_CAP_BAROMETER){
    return sensor->pressure;
  }
  return 0.0;
}

bool mgos_barometer_init(void) {
  return true;
}

// Public functions end
