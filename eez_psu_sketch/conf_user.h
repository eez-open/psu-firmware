/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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
 
/** @file conf_user.h
@brief Configuration overrided by the user.
Use this header file to override anything from conf.h or conf_advanced.h.
*/

#pragma once


/// Channels configuration CH_BOARD_REVISION_R5B6B_PARAMS or CH_BOARD_REVISION_R4B43A_PARAMS

#undef CHANNELS
#define CHANNELS \
    CHANNEL(1, CH_BOARD_REVISION_R5B6B_PARAMS, CH_PINS_1, CH_PARAMS_40V_5A), \
    CHANNEL(2, CH_BOARD_REVISION_R5B6B_PARAMS, CH_PINS_2, CH_PARAMS_40V_5A) \

#undef OPTION_ETHERNET
#define OPTION_ETHERNET   0

/// PSU serial number.
#undef PSU_SERIAL
#define PSU_SERIAL   "00003"

/// Firmware version.
#undef FIRMWARE
#define FIRMWARE     "M2.1.16 (20160719)"

/// Select type of TFT touch display, possible values are: TFT_320QVT_1289 and TFT_320QVT_9341
#undef DISPLAY_TYPE
#define DISPLAY_TYPE TFT_320QVT_9341

/// Number of channels visible (less then or equal to CH_MAX)
#undef CH_NUM
#define CH_NUM 1

/// Set to 1 to skip the test of PWRGOOD signal
//#undef CONF_SKIP_PWRGOOD_TEST
//#define CONF_SKIP_PWRGOOD_TEST 0

/// This is the delay period, after the channel output went OFF,
/// after which we shall turn DP off.
/// Value is given in seconds.
#undef DP_OFF_DELAY_PERIOD
#define DP_OFF_DELAY_PERIOD 1

/// Is main temperature sensor present?
#undef OPTION_MAIN_TEMP_SENSOR
#define OPTION_MAIN_TEMP_SENSOR 0

/// Set to 1 to skip the test of PWRGOOD signal
//#undef CONF_SKIP_PWRGOOD_TEST
//#define CONF_SKIP_PWRGOOD_TEST 1
