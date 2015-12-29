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

/// Wait until serial port is ready before starting firmware.
#define CONF_WAIT_SERIAL  0

/// Enable all debug trace to the serial port. 
#define CONF_DEBUG        1

/// Enable only some of the debug trace to the serial port. 
#define CONF_DEBUG_LATEST 1

/// PSU serial number.
#define PSU_SERIAL   "123456789"

/// Firmware version.
#define FIRMWARE     "M1"

/// Manufacturer description text.
#define MANUFACTURER "EEZ"

/// SCPI TCP server port.
#define TCP_PORT 5025

/// Name of the DAC chip.
#define DAC_NAME "DAC8552"

/// DAC chip resolution in number of bits.
#define DAC_RES 16

/// Allowed difference, in percentage, between DAC and ADC value during testing.
#define DAC_TEST_TOLERANCE 4.0f

/// Max. number of tries during DAC testing before giving up. 
#define DAC_TEST_MAX_TRIES 3

/// Name of the ADC chip.
#define ADC_NAME "ADS1120"

/// ADC chip resolution in number of bits.
#define ADC_RES 15

/// How many times per second will ADC take snapshot value?
/// 0: 20 SPS, 1: 45 SPS, 2: 90 SPS, 3: 175 SPS, 4: 330 SPS, 5: 600 SPS, 6: 1000 SPS
#define ADC_SPS 5

/// Duration, in milliseconds, from the last ADC interrupt
/// after which ADC timeout condition is declared.  
#define ADC_TIMEOUT_MS 60

/// Maximum number of attempts to recover from ADC timeout before giving up.
#define MAX_ADC_TIMEOUT_RECOVERY_ATTEMPTS 3

/// Password minimum length in number characters.
#define PASSWORD_MIN_LENGTH 4

/// Password maximum length in number of characters.
#define PASSWORD_MAX_LENGTH 16

/// Calibration remark maximum length.
#define CALIBRATION_REMARK_MAX_LENGTH 32

/// Default calibration remark text.
#define CALIBRATION_REMARK_INIT "Not calibrated"

/// Maximum difference, in percentage, between ADC
/// and real value during calibration.
#define CALIBRATION_DATA_TOLERANCE 5.0f

/// Maximum difference, in percentage, between calculated mid value
/// and real mid value during calibration.
#define CALIBRATION_MID_TOLERANCE_PERCENT 1.0f

/// Number of digits after decimal point
/// in float to string conversion.
#define FLOAT_TO_STR_PREC 2

/// Temperature reading interval.
#define TEMP_SENSOR_READ_EVERY_MS 1000

/// Voltage value of the first temperature calibration point 
#define MAIN_TEMP_COEF_P1_U 1.6f

/// Temperature(oC) value of the first temperature calibration point 
#define MAIN_TEMP_COEF_P1_T 25.0f

/// Voltage value of the second temperature calibration point 
#define MAIN_TEMP_COEF_P2_U 4.07f

/// Temperature(oC) value of the second temperature calibration point 
#define MAIN_TEMP_COEF_P2_T 85.0f

/// Minimum OTP delay
#define OTP_MAIN_MIN_DELAY     0.0f

/// Maximum OTP delay
#define OTP_MAIN_MAX_DELAY     300.0f

/// Minimum OTP level
#define OTP_MAIN_MIN_LEVEL     0.0f

/// Maximum OTP level
#define OTP_MAIN_MAX_LEVEL     100.0f

/// Number of profile storage locations
#define NUM_PROFILE_LOCATIONS 10

/// Profile name maximum length in number of characters.
#define PROFILE_NAME_MAX_LENGTH 32

/// Size in number characters of SCPI parser input buffer.
#define SCPI_PARSER_INPUT_BUFFER_LENGTH 48

/// Size of SCPI parser error queue.
#define SCPI_PARSER_ERROR_QUEUE_SIZE 20

/// Since we are not using timer, but ADC interrupt for the OVP and
/// OCP delay measuring there will be some error (size of which
/// depends on ADC_SPS value). You can use the following value, which
/// will be subtracted from the OVP and OCP delay, to correct this error.
/// Value is given in seconds.
#define PROT_DELAY_CORRECTION 0.002f

/// This is the delay period, after the channel output went OFF,
/// after which we shall turn DP off.
/// Value is given in seconds.
#define DP_OFF_DELAY_PERIOD 0.05

/// Text returned by the SYStem:CAPability command
#define STR_SYST_CAP "DCSUPPLY WITH (MEASURE|MULTIPLE|TRIGGER)"
