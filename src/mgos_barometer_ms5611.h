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

#define MS5611_PROM_SIZE       8

#define MS5611_CMD_RESET       (0x1E)     // ADC reset command
#define MS5611_CMD_ADC_READ    (0x00)     // ADC read command
#define MS5611_CMD_ADC_CONV    (0x40)     // ADC conversion command
#define MS5611_CMD_ADC_D1      (0x00)     // ADC D1 conversion
#define MS5611_CMD_ADC_D2      (0x10)     // ADC D2 conversion
#define MS5611_CMD_ADC_256     (0x00)     // ADC OSR=256
#define MS5611_CMD_ADC_512     (0x02)     // ADC OSR=512
#define MS5611_CMD_ADC_1024    (0x04)     // ADC OSR=1024
#define MS5611_CMD_ADC_2048    (0x06)     // ADC OSR=2048
#define MS5611_CMD_ADC_4096    (0x08)     // ADC OSR=4096
#define MS5611_CMD_PROM_RD     (0xA0)     // Prom read command

struct mgos_barometer_ms5611_data {
  // Calibration data:
  // 16 bits -- factory code
  // 6x 16 bits of calibration (c1..c6)
  // last 16 bits -- crc4 of the ROM in LSB4, other 12 bits are ignored
  uint16_t calib[MS5611_PROM_SIZE];
};

bool mgos_barometer_ms5611_create(struct mgos_barometer *dev);
bool mgos_barometer_ms5611_destroy(struct mgos_barometer *dev);
bool mgos_barometer_ms5611_read(struct mgos_barometer *dev);
