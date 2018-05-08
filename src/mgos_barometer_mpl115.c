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

#include "mgos_barometer_mpl115.h"
#include "mgos_i2c.h"

// Datasheet:
// https://cdn-shop.adafruit.com/datasheets/MPL115A2.pdf

bool mgos_barometer_mpl115_create(struct mgos_barometer *dev) {
  struct mgos_barometer_mpl115_data *mpl115_data;

  if (!dev) {
    return false;
  }
  uint8_t data[8];
  if (!mgos_i2c_read_reg_n(dev->i2c, dev->i2caddr, MPL115_REG_COEFF_BASE, 8, data)) {
    return false;
  }

  mpl115_data = calloc(1, sizeof(struct mgos_barometer_mpl115_data));
  if (!mpl115_data) {
    return false;
  }

  // TESTDATA:
  // data[0]=0x3e; data[1]=0xce; data[2]=0xb3; data[3]=0xf9; data[4]=0xc5; data[5]=0x17; data[6]=0x33; data[7]=0xc8;
  int16_t a0  = ((uint16_t)data[0] << 8) | data[1];
  int16_t b1  = ((uint16_t)data[2] << 8) | data[3];
  int16_t b2  = ((uint16_t)data[4] << 8) | data[5];
  int16_t c12 = (((uint16_t)data[6] << 8) | data[7]) >> 2;

  mpl115_data->a0  = (float)a0 / (1 << 3);
  mpl115_data->b1  = (float)b1 / (1 << 13);
  mpl115_data->b2  = (float)b2 / (1 << 14);
  mpl115_data->c12 = (float)c12 / (1 << 22);
  dev->user_data   = mpl115_data;

  dev->capabilities |= MGOS_BAROMETER_CAP_BAROMETER;
  dev->capabilities |= MGOS_BAROMETER_CAP_THERMOMETER;
  return true;
}

bool mgos_barometer_mpl115_destroy(struct mgos_barometer *dev) {
  if (!dev) {
    return false;
  }
  if (dev->user_data) {
    free(dev->user_data);
    dev->user_data = NULL;
  }
  return true;
}

bool mgos_barometer_mpl115_read(struct mgos_barometer *dev) {
  struct mgos_barometer_mpl115_data *mpl115_data;
  int16_t Padc, Tadc;
  float   Pcomp;

  if (!dev) {
    return false;
  }
  mpl115_data = (struct mgos_barometer_mpl115_data *)dev->user_data;
  if (!mpl115_data) {
    return false;
  }

  if (!mgos_i2c_write_reg_b(dev->i2c, dev->i2caddr, MPL115_REG_START, 0x00)) {
    return false;
  }

  mgos_usleep(4000);
  uint8_t data[4];
  if (!mgos_i2c_read_reg_n(dev->i2c, dev->i2caddr, MPL115_REG_PRESSURE, 4, data)) {
    return false;
  }
  // TESTDATA:
  // data[0]=0x66; data[1]=0x80; data[2]=0x7e; data[3]=0xc0;
  Padc = (((uint16_t)data[0] << 8) | data[1]) >> 6;
  Tadc = (((uint16_t)data[2] << 8) | data[3]) >> 6;

  Pcomp = mpl115_data->a0 + (mpl115_data->b1 + mpl115_data->c12 * Tadc) * Padc + mpl115_data->b2 * Tadc;

  dev->pressure    = (Pcomp * (65.0 / 1023) + 50.0) * 1000;   // Pascals
  dev->temperature = ((float)Tadc - 498.0F) / -5.35F + 25.0F; // Celsius

  /* TESTDATA (in mpl115_data and Padc/Tadc) yields:
   * pressure=96587.33
   * temperature=23.32
   */

  return true;
}
