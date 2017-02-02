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
 
#pragma once

#ifdef	__cplusplus
extern "C" {
#endif

#define USE_COMMAND_TAGS 0

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define USE_64K_PROGMEM_FOR_CMD_LIST 1
#define USE_FULL_PROGMEM_FOR_CMD_LIST 0
#define SCPI_MAX_CMD_PATTERN_SIZE 128
#define USE_64K_PROGMEM_FOR_ERROR_MESSAGES 1
#define USE_FULL_PROGMEM_FOR_ERROR_MESSAGES 0
#define SCPI_MAX_ERROR_MESSAGE_SIZE 64
#endif

#define USE_USER_ERROR_LIST 1
#define LIST_OF_USER_ERRORS \
    X(SCPI_ERROR_DATA_OUT_OF_RANGE,                         -222, "Data out of range")                            \
    X(SCPI_ERROR_TOO_MUCH_DATA,                             -223, "Too much data")                                \
    X(SCPI_ERROR_HARDWARE_ERROR,                            -240, "Hardware error")                               \
    X(SCPI_ERROR_CH1_FAULT_DETECTED,                        -242, "CH1 fault detected")                           \
	X(SCPI_ERROR_CH2_FAULT_DETECTED,                        -243, "CH2 fault detected")                           \
    X(SCPI_ERROR_CH1_OUTPUT_FAULT_DETECTED,                 -245, "CH1 output fault detected")                    \
	X(SCPI_ERROR_CH2_OUTPUT_FAULT_DETECTED,                 -246, "CH2 output fault detected")                    \
    X(SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE,                  -114, "Header suffix out of range")                   \
    X(SCPI_ERROR_CHANNEL_NOT_FOUND,                          100, "Channel not found")                            \
    X(SCPI_ERROR_CALIBRATION_STATE_IS_OFF,                   101, "Calibration state is off")                     \
    X(SCPI_ERROR_INVALID_SYS_PASSWORD,                       122, "Invalid sys password")                         \
    X(SCPI_ERROR_SYS_PASSWORD_TOO_LONG,                      125, "Sys password too long")                        \
    X(SCPI_ERROR_SYS_PASSWORD_TOO_SHORT,                     126, "Sys password too short")                       \
    X(SCPI_ERROR_INVALID_CAL_PASSWORD,                       102, "Invalid cal password")                         \
    X(SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS,       104, "Bad sequence of calibration commands")         \
    X(SCPI_ERROR_CAL_PASSWORD_TOO_LONG,                      105, "Cal password too long")                        \
    X(SCPI_ERROR_CAL_PASSWORD_TOO_SHORT,                     106, "Cal password too short")                       \
    X(SCPI_ERROR_CAL_VALUE_OUT_OF_RANGE,                     107, "Cal value out of range")                       \
    X(SCPI_ERROR_CAL_OUTPUT_DISABLED,                        108, "Cal output disabled")                          \
    X(SCPI_ERROR_INVALID_CAL_DATA,                           109, "Invalid cal data")                             \
    X(SCPI_ERROR_CAL_PARAMS_MISSING,                         110, "Cal params missing or corrupted")              \
    X(SCPI_ERROR_POWER_LIMIT_EXCEEDED,                       150, "Power limit exceeded")                         \
	X(SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED,                     151, "Voltage limit exceeded")                       \
	X(SCPI_ERROR_CURRENT_LIMIT_EXCEEDED,                     152, "Current limit exceeded")                       \
    X(SCPI_ERROR_CANNOT_EXECUTE_BEFORE_CLEARING_PROTECTION,  201, "Cannot execute before clearing protection")    \
    X(SCPI_ERROR_CH1_IOEXP_TEST_FAILED,                      210, "CH1 IOEXP test failed")                        \
    X(SCPI_ERROR_CH2_IOEXP_TEST_FAILED,                      211, "CH2 IOEXP test failed")                        \
    X(SCPI_ERROR_CH1_ADC_TEST_FAILED,                        220, "CH1 ADC test failed")                          \
    X(SCPI_ERROR_CH2_ADC_TEST_FAILED,                        221, "CH2 ADC test failed")                          \
    X(SCPI_ERROR_CH1_DAC_TEST_FAILED,                        230, "CH1 DAC test failed")                          \
    X(SCPI_ERROR_CH2_DAC_TEST_FAILED,                        231, "CH2 DAC test failed")                          \
    X(SCPI_ERROR_EXT_EEPROM_TEST_FAILED,                     240, "External EEPROM test failed")                  \
    X(SCPI_ERROR_RTC_TEST_FAILED,                            250, "RTC test failed")                              \
    X(SCPI_ERROR_ETHERNET_TEST_FAILED,                       260, "Ethernet test failed")                         \
    X(SCPI_ERROR_CH1_ADC_TIMEOUT_DETECTED,                   270, "CH1 ADC timeout detected")                     \
    X(SCPI_ERROR_CH2_ADC_TIMEOUT_DETECTED,                   271, "CH2 ADC timeout detected")                     \
    X(SCPI_ERROR_OPTION_NOT_INSTALLED,                       302, "Option not installed")                         \
	X(SCPI_ERROR_FAN_TEST_FAILED,                            630, "Fan test failed")                              \
	X(SCPI_ERROR_AUX_TEMP_SENSOR_TEST_FAILED,                720, "AUX temperature sensor test failed")           \
    X(SCPI_ERROR_CH1_TEMP_SENSOR_TEST_FAILED,                722, "CH1 temperature sensor test failed")           \
    X(SCPI_ERROR_CH2_TEMP_SENSOR_TEST_FAILED,                723, "CH2 temperature sensor test failed")           \
    X(SCPI_ERROR_CHARACTER_DATA_TOO_LONG,                   -144, "Character data too long")                      \
    X(SCPI_ERROR_CH1_DOWN_PROGRAMMER_SWITCHED_OFF,           500, "Down-programmer on CH1 switched off")          \
    X(SCPI_ERROR_CH2_DOWN_PROGRAMMER_SWITCHED_OFF,           501, "Down-programmer on CH2 switched off")          \
    X(SCPI_ERROR_EXECUTE_ERROR_CHANNELS_ARE_COUPLED,         312, "Cannot execute when the channels are coupled") \
    X(SCPI_ERROR_EXECUTE_ERROR_IN_TRACKING_MODE,             313, "Cannot execute in tracking mode")              \
    X(SCPI_ERROR_TRIGGER_IGNORED,                           -211, "Trigger ignored")                              \

// strtoull is not defined on some arduino boards
// TODO mvladic:find better way to do this
#ifndef strtoull
#define strtoull strtoul
#endif

// strtoll is not defined on some arduino boards
// TODO mvladic:find better way to do this
#ifndef strtoll
#define strtoll strtol
#endif

#ifdef _MSC_VER 
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#ifdef	__cplusplus
}
#endif
