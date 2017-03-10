/*
 * EEZ PSU Firmware
 * Copyright (C) 2015-present, Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
/** @file conf_example.h
@brief Compile time configuration.
*/

/** @page conf_h Configuration
@brief Compile time configuration.
This file is used to define compile time configuration options.
Use `conf_user.h` file to override anything from here.
option.
*/

#pragma once

#include "conf_user_revision.h"

#include "conf_channel.h"

/// Wait until serial port is ready before starting firmware.
#define CONF_WAIT_SERIAL 0

// Data rate in bits per second (baud) for serial data transmission.
#define SERIAL_SPEED 9600

/// Enable all debug trace to the serial port. 
#define CONF_DEBUG 1

/// Enable only some of the debug trace to the serial port. 
#define CONF_DEBUG_LATEST 1

/// Enable debug variables
#define CONF_DEBUG_VARIABLES 0

/// Is Ethernet present?
#define OPTION_ETHERNET 1

/// Is RTC present?
#define OPTION_EXT_RTC 1

/// Is SD card present?
#define OPTION_SD_CARD 1

/// Is external EEPROM present?
#define OPTION_EXT_EEPROM 1

/// Is binding post present?
#define OPTION_BP 1

/// Is display present?
#define OPTION_DISPLAY 1

/// Is fan present?
#define OPTION_FAN 1

/// Is aux temperature sensor present?
#define OPTION_AUX_TEMP_SENSOR 1

/// Generate square wave on SYNC_MASTER pin
#define OPTION_SYNC_MASTER 1

/// Enable watchdog
#define OPTION_WATCHDOG 1

/// Enable encoder
#define OPTION_ENCODER 1

/// Maximum number of channels existing.
#define CH_MAX 2

/// Number of channels visible (less then or equal to CH_MAX)
#define CH_NUM 2

/// Channels configuration.
/// 
#define CHANNELS \
    CHANNEL(1, CH_BOARD_REVISION_R5B9_PARAMS, CH_PINS_1, CH_PARAMS_40V_5A), \
    CHANNEL(2, CH_BOARD_REVISION_R5B9_PARAMS, CH_PINS_2, CH_PARAMS_40V_5A) \

/// Min. delay between power down and power up.
#define MIN_POWER_UP_DELAY 5

/// Default calibration password.
#define CALIBRATION_PASSWORD_DEFAULT "eezpsu"

/// Is OTP enabled by default?
#define OTP_AUX_DEFAULT_STATE 1

/// Default OTP delay
#define OTP_AUX_DEFAULT_DELAY 10.0f

/// Default OTP level
#define OTP_AUX_DEFAULT_LEVEL 50.0f

/// Is channel OTP enabled by default?
#define OTP_CH_DEFAULT_STATE 1

/// Default channel OTP delay
#define OTP_CH_DEFAULT_DELAY 30.0f

/// Default channel OTP level
#define OTP_CH_DEFAULT_LEVEL 75.0f

#include "conf_advanced.h"
#include "conf_user.h"
