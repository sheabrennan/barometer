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
