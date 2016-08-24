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


#undef OPTION_ETHERNET
#define OPTION_ETHERNET   0

/// PSU serial number.
#undef PSU_SERIAL
#define PSU_SERIAL   "00003"

/// Firmware version.
#undef FIRMWARE
#define FIRMWARE     "M2.1.22 (20160824)"

/// Select type of TFT touch display, possible values are: TFT_320QVT_1289 and TFT_320QVT_9341
#undef DISPLAY_TYPE
#define DISPLAY_TYPE TFT_320QVT_9341

/// Set to 1 to skip the test of PWRGOOD signal
//#undef CONF_SKIP_PWRGOOD_TEST
//#define CONF_SKIP_PWRGOOD_TEST 0

/// This is the delay period, after the channel output went OFF,
/// after which we shall turn DP off.
/// Value is given in seconds.
#undef DP_OFF_DELAY_PERIOD
#define DP_OFF_DELAY_PERIOD 30

/// Is main temperature sensor present?
#undef OPTION_MAIN_TEMP_SENSOR
#define OPTION_MAIN_TEMP_SENSOR 0

/// Set to 1 to skip the test of PWRGOOD signal
//#undef CONF_SKIP_PWRGOOD_TEST
//#define CONF_SKIP_PWRGOOD_TEST 1

//#undef CH_NUM
//#define CH_NUM 1
