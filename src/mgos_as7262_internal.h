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

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mgos.h"
#include "mgos_as7262.h"

#define MGOS_AS7262_I2C_ADDR 0x49

// Harware registers
#define MGOS_AS7262_REG_STATUS 0x00
#define MGOS_AS7262_REG_WRITE 0x01
#define MGOS_AS7262_REG_READ 0x02

// Virtual registers
#define MGOS_AS7262_VREG_HW_TYPE 0x00
#define MGOS_AS7262_VREG_HW_VERSION 0x01
#define MGOS_AS7262_VREG_FW_VERSION_H 0x02
#define MGOS_AS7262_VREG_FW_VERSION_L 0x03
#define MGOS_AS7262_VREG_CTRL_SETUP 0x04
#define MGOS_AS7262_VREG_CTRL_INT_T 0x05
#define MGOS_AS7262_VREG_CTRL_TEMP 0x06
#define MGOS_AS7262_VREG_CTRL_LED 0x07
#define MGOS_AS7262_VREG_RAW_CHAN 0x08  // 2 bytes wide, stores 16bit unsigned int; MSB,LSB
#define MGOS_AS7262_VREG_RAW_CHAN_V 0x08
#define MGOS_AS7262_VREG_RAW_CHAN_B 0x0a
#define MGOS_AS7262_VREG_RAW_CHAN_G 0x0c
#define MGOS_AS7262_VREG_RAW_CHAN_Y 0x0e
#define MGOS_AS7262_VREG_RAW_CHAN_O 0x10
#define MGOS_AS7262_VREG_RAW_CHAN_R 0x12
#define MGOS_AS7262_VREG_CAL_CHAN 0x14  // 4 bytes wide, stores 32bit float.
#define MGOS_AS7262_VREG_CAL_CHAN_V 0x14
#define MGOS_AS7262_VREG_CAL_CHAN_B 0x18
#define MGOS_AS7262_VREG_CAL_CHAN_G 0x1c
#define MGOS_AS7262_VREG_CAL_CHAN_Y 0x20
#define MGOS_AS7262_VREG_CAL_CHAN_O 0x24
#define MGOS_AS7262_VREG_CAL_CHAN_R 0x28

struct mgos_as7262 {
  struct mgos_i2c *i2c;
  struct mgos_as7262_config cfg;
};
