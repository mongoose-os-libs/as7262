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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "mgos_i2c.h"

struct mgos_as7262;

enum mgos_as7262_gain { GAIN_1X = 0, GAIN_3X7, GAIN_16X, GAIN_64X };

enum mgos_as7262_mode { MODE_CONT_VBGY = 0, MODE_CONT_GYOR, MODE_CONT_ALL, MODE_ONESHOT_ALL };

enum mgos_as7262_chan { CHAN_VIOLET = 0, CHAN_BLUE, CHAN_GREEN, CHAN_YELLOW, CHAN_ORANGE, CHAN_RED };

struct mgos_as7262_config {
  uint8_t i2caddr;
  enum mgos_as7262_gain gain;
  enum mgos_as7262_mode mode;
  int gpio_int;
  uint8_t integration_time;
};

// Create an instance of the driver with the given configuration struct, which may be NULL
// in which case reasonable defaults are chosen.
//
// Returns a pointer to the object upon success, NULL otherwise.
struct mgos_as7262 *mgos_as7262_create(struct mgos_i2c *i2c, const struct mgos_as7262_config *cfg);

// Set sensor gain (for all channels).
//
// Returns false upon error, true otherwise.
bool mgos_as7262_set_gain(struct mgos_as7262 *d, const enum mgos_as7262_gain gain);

// Set sensor mode. There are four modes:
// - MODE_CONT_VBGY: Continuously measure violet, blue, green, yellow channels.
// - MODE_CONT_GYOR: Continuously measure green, yellow, orange and red channels.
// - MODE_CONT_ALL: Continuously measure all channels, but data will be
//                  available once every two integration time periods.
// - MODE_ONESHOT_ALL: Take a one-off measurement of all channels.
//
// Returns false upon error, true otherwise.
bool mgos_as7262_set_mode(struct mgos_as7262 *d, const enum mgos_as7262_mode mode);

// Set sensor integration time. The value is in 2.8ms increments, so setting a value of 36
// will yield an integration time of 100.8ms. Note that for continus all-channel or oneshot
// measurements, the data will be available in two cycles (ie. 201.6ms in the default case).
//
// Returns false upon error, true otherwise.
bool mgos_as7262_set_integration_time(struct mgos_as7262 *d, const uint8_t t);

// Return the sensor gain (for all channels) in *gain.
//
// Returns false upon error, true otherwise.
bool mgos_as7262_get_gain(struct mgos_as7262 *d, enum mgos_as7262_gain *gain);

// Return the current sensor mode in *mode.
//
// Returns false upon error, true otherwise.
bool mgos_as7262_get_mode(struct mgos_as7262 *d, enum mgos_as7262_mode *mode);

// Return the current integration time mode in *t, which is in multiples of 2.8ms. A returned
// value of '36' means an integration time of 36 * 2.8ms == 100.8ms (the default).
//
// Returns false upon error, true otherwise.
bool mgos_as7262_get_integration_time(struct mgos_as7262 *d, uint8_t *t);

// Return the last integrated measurement of the specified channel. Higher values means more
// light was received in that channel. Valid channels are CHAN_VIOLET through CHAN_RED.
//
// Returns false upon error, true otherwise.
bool mgos_as7262_get_channel(struct mgos_as7262 *d, const enum mgos_as7262_chan chan, float *value);
bool mgos_as7262_get_channel_raw(struct mgos_as7262 *d, const enum mgos_as7262_chan chan, uint16_t *value);

// Clean up the driver and return memory used for it.
//
// Returns false upon error, true otherwise.
bool mgos_as7262_destroy(struct mgos_as7262 **dev);

#ifdef __cplusplus
}
#endif
