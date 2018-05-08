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

#include "mgos_barometer_ms5611.h"
#include "mgos_i2c.h"

// Datasheet:
// http://www.amsys.info/sheets/amsys.en.ms5611_01ba03.pdf

static bool ms5611_crc4(uint16_t *data) {
  int32_t i, j;
  uint32_t res = 0;
  uint8_t crc = data[7] & 0xF;
  data[7] &= 0xFF00;

  bool blankEeprom = true;

  for (i = 0; i < 16; i++) {
    if (data[i >> 1]) {
      blankEeprom = false;
    }
    if (i & 1)
      res ^= ((data[i >> 1]) & 0x00FF);
    else
      res ^= (data[i >> 1] >> 8);
    for (j = 8; j > 0; j--) {
      if (res & 0x8000)
        res ^= 0x1800;
      res <<= 1;
    }
  }
  data[7] |= crc;
  if (!blankEeprom && crc == ((res >> 12) & 0xF))
    return true;

  return false;
}
static bool ms5611_conv(struct mgos_barometer *dev, uint8_t cmd, uint32_t *conv) {
  uint8_t data[3];

  if (!dev) return false;
  if (!mgos_i2c_write(dev->i2c, dev->i2caddr, &cmd, 1, true))
    return false;

  switch (cmd & 0x0f) {
    case MS5611_CMD_ADC_256 : mgos_usleep(900); break;
    case MS5611_CMD_ADC_512 : mgos_usleep(3000); break;
    case MS5611_CMD_ADC_1024: mgos_usleep(4000); break;
    case MS5611_CMD_ADC_2048: mgos_usleep(6000); break;
    default: mgos_usleep(10000); break;
  }
  if (!mgos_i2c_read_reg_n(dev->i2c, dev->i2caddr, MS5611_CMD_ADC_READ, 3, data))
    return false;

  if (*conv)
    *conv = (((uint32_t)data[0]) << 16) | ((uint32_t)(data[1]) << 8) | data[2];
  return true;
}

bool mgos_barometer_ms5611_create(struct mgos_barometer *dev) {
  struct mgos_barometer_ms5611_data *ms5611_data;

  if (!dev) {
    return false;
  }

  ms5611_data = calloc(1, sizeof(struct mgos_barometer_ms5611_data));
  if (!ms5611_data) {
    return false;
  }
  dev->user_data = ms5611_data;

  // Reset device
  uint8_t cmd = MS5611_CMD_RESET;
  if (!mgos_i2c_write(dev->i2c, dev->i2caddr, &cmd, 1, true))
    return false;
  mgos_usleep(3000);

  // Read calibration coefficients from PROM
  for(int i=0; i<MS5611_PROM_SIZE; i++) {
    int val=mgos_i2c_read_reg_w(dev->i2c, dev->i2caddr, MS5611_CMD_PROM_RD+i*2);
    if (val<0) return false;
    ms5611_data->calib[i] = val;
  }
  if (!ms5611_crc4(ms5611_data->calib)) {
    LOG(LL_ERROR, ("CRC4 failure on PROM data"));
    return false;
  }

  dev->capabilities |= MGOS_BAROMETER_CAP_BAROMETER;
  dev->capabilities |= MGOS_BAROMETER_CAP_THERMOMETER;

  return true;
}

bool mgos_barometer_ms5611_destroy(struct mgos_barometer *dev) {
  if (!dev) {
    return false;
  }
  if (dev->user_data) {
    free(dev->user_data);
    dev->user_data = NULL;
  }
  return true;
}

bool mgos_barometer_ms5611_read(struct mgos_barometer *dev) {
  struct mgos_barometer_ms5611_data *ms5611_data;

  if (!dev) {
    return false;
  }
  ms5611_data = (struct mgos_barometer_ms5611_data *)dev->user_data;
  if (!ms5611_data) {
    return false;
  }

  uint32_t Tadc, Padc;
  if (!ms5611_conv(dev, MS5611_CMD_ADC_CONV | MS5611_CMD_ADC_D2 | MS5611_CMD_ADC_4096, &Tadc)) {
    LOG(LL_ERROR, ("Could not read temperature ADC"));
    return false;
  }
  if (!ms5611_conv(dev, MS5611_CMD_ADC_CONV | MS5611_CMD_ADC_D1 | MS5611_CMD_ADC_4096, &Padc)) {
    LOG(LL_ERROR, ("Could not read pressure ADC"));
    return false;
  }
  LOG(LL_DEBUG, ("Padc=%u Tadc=%u", Padc, Tadc));

  // Convert ADC to values -- TODO(pim) check this math
//  uint32_t press;
  int64_t temp;
  int64_t delt;
  int64_t dT = (int64_t)Tadc - ((uint64_t)ms5611_data->calib[5] * 256);
  int64_t off = ((int64_t)ms5611_data->calib[2] << 16) + (((int64_t)ms5611_data->calib[4] * dT) >> 7);
  int64_t sens = ((int64_t)ms5611_data->calib[1] << 15) + (((int64_t)ms5611_data->calib[3] * dT) >> 8);
  temp = 2000 + ((dT * (int64_t)ms5611_data->calib[6]) >> 23);

  if (temp < 2000) { // temperature lower than 20degC
    delt = temp - 2000;
    delt = 5 * delt * delt;
    off -= delt >> 1;
    sens -= delt >> 2;
    if (temp < -1500) { // temperature lower than -15degC
      delt = temp + 1500;
      delt = delt * delt;
      off -= 7 * delt;
      sens -= (11 * delt) >> 1;
    }
    temp -= ((dT * dT) >> 31);
  }

  dev->pressure=((((int64_t)Padc * sens) >> 21) - off) >> 15;
  dev->temperature=(float)temp / 100.0;

  LOG(LL_DEBUG, ("P=%.2f T=%.2f", dev->pressure, dev->temperature));

  return true;
}
