/*
 * Copyright 2020 Pim van Pelt <pim@ipng.nl>
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

#include "mgos_as7262_internal.h"
#include "mgos_i2c.h"

static uint8_t mgos_as7262_i2c_read(struct mgos_as7262 *d, uint8_t reg) {
  uint8_t val;
  if (!d) return 0;
  if (!mgos_i2c_write(d->i2c, d->cfg.i2caddr, &reg, 1, true)) return 0;
  if (!mgos_i2c_read(d->i2c, d->cfg.i2caddr, &val, 1, true)) return 0;
  return val;
}

static void mgos_as7262_i2c_write(struct mgos_as7262 *d, uint8_t reg, uint8_t val) {
  uint8_t buf[2];
  buf[0] = reg;
  buf[1] = val;
  mgos_i2c_write(d->i2c, d->cfg.i2caddr, buf, 2, true);
  return;
}

uint8_t mgos_as7262_virtual_read(struct mgos_as7262 *d, uint8_t reg) {
  uint8_t status, val;
  while (1) {
    status = mgos_as7262_i2c_read(d, MGOS_AS7262_REG_STATUS);
    if ((status & 0x02) == 0) break;
  }
  mgos_as7262_i2c_write(d, MGOS_AS7262_REG_WRITE, reg);
  while (1) {
    status = mgos_as7262_i2c_read(d, MGOS_AS7262_REG_STATUS);
    if ((status & 0x01) != 0) break;
  }
  val = mgos_as7262_i2c_read(d, MGOS_AS7262_REG_READ);
  return val;
}

void mgos_as7262_virtual_write(struct mgos_as7262 *d, uint8_t reg, uint8_t val) {
  uint8_t status;
  while (1) {
    status = mgos_as7262_i2c_read(d, MGOS_AS7262_REG_STATUS);
    if ((status & 0x02) == 0) break;
  }
  mgos_as7262_i2c_write(d, MGOS_AS7262_REG_WRITE, reg | 0x80);
  while (1) {
    status = mgos_as7262_i2c_read(d, MGOS_AS7262_REG_STATUS);
    if ((status & 0x02) == 0) break;
  }
  mgos_as7262_i2c_write(d, MGOS_AS7262_REG_WRITE, val);
  return;
}

void mgos_as7262_default_cfg(struct mgos_as7262_config *cfg) {
  if (!cfg) return;
  cfg->i2caddr = MGOS_AS7262_I2C_ADDR;
  cfg->mode = MODE_CONT_ALL;
  cfg->gain = GAIN_64X;
  cfg->gpio_int = -1;
  cfg->integration_time = 36;
}

struct mgos_as7262 *mgos_as7262_create(struct mgos_i2c *i2c, const struct mgos_as7262_config *in_cfg) {
  struct mgos_as7262 *d;
  uint8_t val, version;
  if (!i2c) return NULL;
  if (!(d = calloc(1, sizeof(*d)))) return NULL;
  d->i2c = i2c;

  if (!in_cfg)
    mgos_as7262_default_cfg(&d->cfg);
  else
    memcpy(&d->cfg, in_cfg, sizeof(*in_cfg));

  uint8_t type = mgos_as7262_virtual_read(d, MGOS_AS7262_VREG_HW_TYPE);
  if (type != 0x40) {
    LOG(LL_ERROR, ("Could not detect AS7262 at I2C address 0x%02x", d->cfg.i2caddr));
    free(d);
    return NULL;
  }

  version = mgos_as7262_virtual_read(d, MGOS_AS7262_VREG_HW_VERSION);
  if (version == 0) {
    LOG(LL_ERROR, ("Could not detect AS7262 at I2C address 0x%02x", d->cfg.i2caddr));
    free(d);
    return NULL;
  }

  // Reset
  mgos_as7262_virtual_write(d, MGOS_AS7262_VREG_CTRL_SETUP, 0x80);
  mgos_usleep(12e5);  // 1200ms

  // Setup: 7=RST 6=INT 54=GAIN 32=BANK(MODE) 1=RDY 0=RESV
  val = 0x00;
  if (d->cfg.gpio_int >= 0) val |= 1 << 6;
  val |= (d->cfg.gain & 0x03) << 4;
  val |= (d->cfg.mode & 0x03) << 2;
  mgos_as7262_virtual_write(d, MGOS_AS7262_VREG_CTRL_SETUP, val);

  mgos_as7262_set_integration_time(d, d->cfg.integration_time);  // 36*2.8ms == 100.8ms

  // LED control: 76=RESV 54=ICL_DRV 3=LED_DRV 21=ICL_IND 0=LED_IND
  // DRV 25mA off; IND 4mA on.
  val = 0b00010101;
  mgos_as7262_virtual_write(d, MGOS_AS7262_VREG_CTRL_LED, val);

  LOG(LL_INFO, ("Initialized AS7262 (version 0x%02x) at I2C address 0x%02x", version, d->cfg.i2caddr));

  return d;
}

bool mgos_as7262_set_integration_time(struct mgos_as7262 *d, const uint8_t val) {
  if (!d) return false;
  d->cfg.integration_time = val;
  mgos_as7262_virtual_write(d, MGOS_AS7262_VREG_CTRL_INT_T, val);
  return true;
}

bool mgos_as7262_get_integration_time(struct mgos_as7262 *d, uint8_t *val) {
  if (!d || !val) return false;
  *val = d->cfg.integration_time;
  return true;
}

bool mgos_as7262_set_gain(struct mgos_as7262 *d, const enum mgos_as7262_gain gain) {
  uint8_t val;
  if (!d) return false;
  val = mgos_as7262_virtual_read(d, MGOS_AS7262_VREG_CTRL_SETUP);
  val &= 0b11001111;
  switch (gain) {
    case GAIN_1X:
      break;
    case GAIN_3X7:
      val |= (1 << 4);
      break;
    case GAIN_16X:
      val |= (2 << 4);
      break;
    case GAIN_64X:
      val |= (3 << 4);
      break;
    default:
      return false;
  }
  d->cfg.gain = gain;
  mgos_as7262_virtual_write(d, MGOS_AS7262_VREG_CTRL_SETUP, val);
  return true;
}

bool mgos_as7262_get_gain(struct mgos_as7262 *d, enum mgos_as7262_gain *gain) {
  if (!d || !gain) return false;
  *gain = d->cfg.gain;
  return true;
}

bool mgos_as7262_set_mode(struct mgos_as7262 *d, const enum mgos_as7262_mode mode) {
  uint8_t val;
  if (!d) return false;
  val = mgos_as7262_virtual_read(d, MGOS_AS7262_VREG_CTRL_SETUP);
  val &= 0b11110011;
  switch (mode) {
    case MODE_CONT_VBGY:
      break;
    case MODE_CONT_GYOR:
      val |= (1 << 2);
      break;
    case MODE_CONT_ALL:
      val |= (2 << 2);
      break;
    case MODE_ONESHOT_ALL:
      val |= (3 << 2);
      break;
    default:
      return false;
  }
  d->cfg.mode = mode;
  mgos_as7262_virtual_write(d, MGOS_AS7262_VREG_CTRL_SETUP, val);
  return true;
}

bool mgos_as7262_get_mode(struct mgos_as7262 *d, enum mgos_as7262_mode *mode) {
  if (!d || !mode) return false;
  *mode = d->cfg.mode;
  return true;
}

bool mgos_as7262_get_channel(struct mgos_as7262 *d, const enum mgos_as7262_chan chan, float *value) {
  if (!d || !value || chan < 0 || chan > CHAN_RED) return false;
  uint32_t val;

  val = ((uint32_t) mgos_as7262_virtual_read(d, MGOS_AS7262_VREG_CAL_CHAN + chan * 4) << 24);
  val |= ((uint32_t) mgos_as7262_virtual_read(d, MGOS_AS7262_VREG_CAL_CHAN + chan * 4 + 1) << 16);
  val |= ((uint32_t) mgos_as7262_virtual_read(d, MGOS_AS7262_VREG_CAL_CHAN + chan * 4 + 2) << 8);
  val |= ((uint32_t) mgos_as7262_virtual_read(d, MGOS_AS7262_VREG_CAL_CHAN + chan * 4 + 3));

  memcpy(value, &val, 4);
  return true;
}

bool mgos_as7262_destroy(struct mgos_as7262 **dev) {
  if (!*dev) return false;
  free(*dev);
  *dev = NULL;
  return true;
}

bool mgos_as7262_init(void) {
  return true;
}
