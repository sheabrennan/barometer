#pragma once

#include "mgos.h"
#include "mgos_barometer_internal.h"

#define MPL3115_REG_STATUS          (0x00)
#define MPL3115_REG_PRESSURE_MSB    (0x01)
#define MPL3115_REG_PRESSURE_CSB    (0x02)
#define MPL3115_REG_PRESSURE_LSB    (0x03)
#define MPL3115_REG_TEMP_MSB        (0x04)
#define MPL3115_REG_TEMP_LSB        (0x05)
#define MPL3115_REG_DR_STATUS       (0x06)
#define MPL3115_REG_WHOAMI          (0x0C)
#define MPL3115_REG_PT_DATA         (0x13)
#define MPL3115_REG_CTRL1           (0x26)
#define MPL3115_REG_CTRL2           (0x27)

bool mgos_barometer_mpl3115_detect(struct mgos_barometer *dev);
bool mgos_barometer_mpl3115_create(struct mgos_barometer *dev);
bool mgos_barometer_mpl3115_read(struct mgos_barometer *dev);
