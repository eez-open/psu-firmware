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
 
/** @file conf_advanced.h
@brief Advanced compile time configuration.
*/

#pragma once

/* 
 * General parameters
 */

/// Number of digits after decimal point
/// in float to string conversion.
#define FLOAT_TO_STR_PREC 2

/// Number of profile storage locations
#define NUM_PROFILE_LOCATIONS 10

/// Profile name maximum length in number of characters
#define PROFILE_NAME_MAX_LENGTH 32

/// Size in number characters of SCPI parser input buffer
#define SCPI_PARSER_INPUT_BUFFER_LENGTH 48

/// Size of SCPI parser error queue
#define SCPI_PARSER_ERROR_QUEUE_SIZE 20

/// Since we are not using timer, but ADC interrupt for measuring 
/// the OVP and OCP delay there will be some error (size of which
/// depends on ADC_SPS value). You can use the following value, which
/// will be subtracted from the programmed OVP and OCP delay, 
/// for better approximation. The value is given in seconds.
#define PROT_DELAY_CORRECTION 0.002f

/// Defines delay in seconds to turn off the down-programmer after
/// the channel output is switched off. That is used to discharge 
/// output capacitor.
#define DP_OFF_DELAY_PERIOD 0.05

/// Text returned by the SYStem:CAPability command
#define STR_SYST_CAP "DCSUPPLY WITH (MEASURE|MULTIPLE|TRIGGER)"

/* 
 * Remote control parameters
 */

/// Wait until serial port is ready before starting firmware
#define CONF_WAIT_SERIAL  0

/// TCP server port for remote control using SCPI commands
#define TCP_PORT 5025

/* 
 * Debug parameters
 */

/// Enable sending of all debug trace to the serial communication interface
#define CONF_DEBUG        1

/// Enable only selected debug trace to the serial communication interface
#define CONF_DEBUG_LATEST 1

/* 
 * PSU identification data
 */

/// PSU serial number that will be shown in results of the *IDN? query
#define PSU_SERIAL   "123456789"

/// Firmware version that will be shown in results of the *IDN? query
#define FIRMWARE     "M1"

/// Manufacturer name that will be shown in results of the *IDN? query
#define MANUFACTURER "EEZ"

/* 
 * SPI peripherals parameters
 */

/// DAC device name for reference only, not currently used
#define DAC_NAME "DAC8552"

/// DAC resolution in number of bits
#define DAC_RES 16

/// Allowed difference in percentage between DAC and 
/// ADC uncalibrated values during self-test
#define DAC_TEST_TOLERANCE 4.0f

/// Number of DAC testing attempts before it’s proclaimed non-operational
#define DAC_TEST_MAX_TRIES 3

/// ADC device name for reference only, not currently used
#define ADC_NAME "ADS1120"

/// ADC resolution in number of bits
#define ADC_RES 15

/// ADC sampling rate
/// 0: 20 SPS, 1: 45 SPS, 2: 90 SPS, 3: 175 SPS, 
/// 4: 330 SPS, 5: 600 SPS, 6: 1000 SPS
#define ADC_SPS 5

/// Max. allowed time in milliseconds between two ADC interrupt 
/// before an ADC timeout condition is declared. The reason for 
/// that could be lost of communication with ADC (e.g. SPI cable 
/// is disconnected) or exceptionally some issue with firmware 
/// that has to be closely inspected if happens repetitively. 
#define ADC_TIMEOUT_MS 60

/// Maximum number of attempts to recover from ADC timeout 
/// condition before giving up and report failure
#define MAX_ADC_TIMEOUT_RECOVERY_ATTEMPTS 3

/* 
 * Calibration parameters
 */

/// Default calibration password
#define CALIBRATION_PASSWORD_DEFAULT "eezpsu"

/// Calibration password minimum length in number of characters
#define PASSWORD_MIN_LENGTH 4

/// Calibration password maximum length in number of characters
#define PASSWORD_MAX_LENGTH 16

/// Calibration remark maximum length
#define CALIBRATION_REMARK_MAX_LENGTH 32

/// Text that will be returned when CALibrate:REMark? query is 
/// executed on channel that is not calibrated yet or calibration 
/// information are erased using CALibration:CLEar
#define CALIBRATION_REMARK_INIT "Not calibrated"

/// Maximum allowed difference, in percentage, between measured 
/// and real (entered by user) value during calibration.
#define CALIBRATION_DATA_TOLERANCE 5.0f

/// Maximum difference, in percentage, between calculated mid value
/// and real mid value during calibration.
#define CALIBRATION_MID_TOLERANCE_PERCENT 1.0f

/* 
 * Temperature sensors and Over-temperature protection (OTP)
 */

/// Temperature reading interval in milliseconds
#define TEMP_SENSOR_READ_EVERY_MS 1000

/// Voltage value of the first temperature calibration point 
#define MAIN_TEMP_COEF_P1_U 1.6f

/// Temperature(oC) value of the first temperature calibration point 
#define MAIN_TEMP_COEF_P1_T 25.0f

/// Voltage value of the second temperature calibration point 
#define MAIN_TEMP_COEF_P2_U 4.07f

/// Temperature(oC) value of the second temperature calibration point 
#define MAIN_TEMP_COEF_P2_T 85.0f

/// Minimum OTP delay in seconds
#define OTP_MAIN_MIN_DELAY     0.0f

/// Maximum OTP delay in seconds
#define OTP_MAIN_MAX_DELAY     300.0f

/// Minimum OTP level in oC
#define OTP_MAIN_MIN_LEVEL     0.0f

/// Maximum OTP level in oC
#define OTP_MAIN_MAX_LEVEL     100.0f

