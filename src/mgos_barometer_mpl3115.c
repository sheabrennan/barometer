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

#include "mgos_barometer_mpl3115.h"
#include "mgos_i2c.h"

// Datasheet:
// https://cdn-shop.adafruit.com/datasheets/1893_datasheet.pdf

bool mgos_barometer_mpl3115_detect(struct mgos_barometer *dev) {
  int val;

  if (!dev) {
    return false;
  }

  if ((val = mgos_i2c_read_reg_b(dev->i2c, dev->i2caddr, MPL3115_REG_WHOAMI)) < 0) {
    return false;
  }
  LOG(LL_DEBUG, ("whoami=0x%02x", val));

  if (val != 0xC4) {
    return false;
  }

  return true;
}

bool mgos_barometer_mpl3115_create(struct mgos_barometer *dev) {
  if (!dev) {
    return false;
  }


  // Reset
  LOG(LL_DEBUG, ("Reset"));
  if (!mgos_i2c_write_reg_b(dev->i2c, dev->i2caddr, MPL3115_REG_CTRL1, 0x04)) {
    return false;
  }
  mgos_usleep(20000);

  // Set sample period to 1sec ST[3:0], period 2^ST seconds
  LOG(LL_DEBUG, ("Sample Period"));
  if (!mgos_i2c_write_reg_b(dev->i2c, dev->i2caddr, MPL3115_REG_CTRL2, 0x00)) {
    return false;
  }

  // Set Barometer Mode, OS[2:0], oversampling 2^OS times, continuous sampling
  LOG(LL_DEBUG, ("Baro Mode"));
  if (!mgos_i2c_write_reg_b(dev->i2c, dev->i2caddr, MPL3115_REG_CTRL1, 0x39)) {
    return false;
  }

  // Set event flags for temp+pressure
  LOG(LL_DEBUG, ("Event Flags"));
  if (!mgos_i2c_write_reg_b(dev->i2c, dev->i2caddr, MPL3115_REG_PT_DATA, 0x07)) {
    return false;
  }

  dev->capabilities|=MGOS_BAROMETER_CAP_BAROMETER;
  dev->capabilities|=MGOS_BAROMETER_CAP_THERMOMETER;

  return true;
}

bool mgos_barometer_mpl3115_read(struct mgos_barometer *dev) {
  if (!dev) {
    return false;
  }

  int val = 0;
  uint8_t retries=100;
  LOG(LL_DEBUG, ("Data Ready"));
  if ((val = mgos_i2c_read_reg_b(dev->i2c, dev->i2caddr, MPL3115_REG_STATUS)) < 0)
    return false;

  while (!(val & 0x08) && retries>0) { // Data Ready
    LOG(LL_DEBUG, ("Data Ready"));
    if ((val = mgos_i2c_read_reg_b(dev->i2c, dev->i2caddr, MPL3115_REG_STATUS)) < 0) {
      return false;
    }
    mgos_usleep(10000);
//    LOG(LL_DEBUG, ("Snoozing, retries=%d", retries));
    retries--;
  }
  if (retries==0) {
    LOG(LL_ERROR, ("Timed out waiting for data ready"));
    return false;
  }

  uint32_t pressure;
  int16_t  temperature;
  uint8_t  data[5];

  LOG(LL_DEBUG, ("Read Data"));
  if (!mgos_i2c_read_reg_n(dev->i2c, dev->i2caddr, MPL3115_REG_PRESSURE_MSB, 5, data)) {
    return false;
  }

  pressure   = data[0];
  pressure <<= 8;
  pressure  |= data[1];
  pressure <<= 8;
  pressure  |= data[2];
  pressure >>= 4;

  temperature   = data[3];
  temperature <<= 8;
  temperature  |= data[4];
  temperature >>= 4;
  if (temperature & 0x800) {
    temperature |= 0xF000;
  }

  dev->pressure    = pressure / 4.0;
  dev->temperature = temperature / 16.0;
  return true;
}
