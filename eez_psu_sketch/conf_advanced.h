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
 
/** @file conf_advanced.h
@brief Advanced compile time configuration.
*/

#pragma once

/// PSU serial number.
#define PSU_SERIAL   "0000000"

/// Firmware version.
#define FIRMWARE     "M5 (WIP)"

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

#define ADC_USE_INTERRUPTS 0

#define ADC_READ_TIME_US 1800

/// How many times per second will ADC take snapshot value?
/// 0: 20 SPS, 1: 45 SPS, 2: 90 SPS, 3: 175 SPS, 4: 330 SPS, 5: 600 SPS, 6: 1000 SPS
#ifdef EEZ_PSU_ARDUINO_MEGA
#define ADC_SPS 2
#else
#define ADC_SPS 5
#endif
#define ADC_SPS_TIME_CRITICAL 5 // used when time/performance critical operation is running

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

/// Temperature reading interval.
#define TEMP_SENSOR_READ_EVERY_MS 1000

/// Temperature sensors calibration points.
/// There are 2 points. For each point there are two values.
/// First value is Voltage[V] and second value is Temperature[oC].
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9

#define AUX_TEMP_SENSOR_CALIBRATION_POINTS 1.6f, 25.0f, 4.07f, 85.0f

#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12

#define AUX_TEMP_SENSOR_CALIBRATION_POINTS 1.6f, 25.0f, 4.07f, 85.0f // 10K, Beta: 3977K
#define CH1_TEMP_SENSOR_CALIBRATION_POINTS 1.6f, 25.0f, 3.89f, 85.0f  // 10K, Beta: 3570K
#define CH2_TEMP_SENSOR_CALIBRATION_POINTS 1.6f, 25.0f, 3.89f, 85.0f  // 10K, Beta: 3570K

#endif

/// Minimum OTP delay
#define OTP_AUX_MIN_DELAY     0.0f

/// Maximum OTP delay
#define OTP_AUX_MAX_DELAY     300.0f

/// Minimum OTP level
#define OTP_AUX_MIN_LEVEL     0.0f

/// Maximum OTP level
#define OTP_AUX_MAX_LEVEL     100.0f

/// Number of profile storage locations
#define NUM_PROFILE_LOCATIONS 10

/// Profile name maximum length in number of characters.
#define PROFILE_NAME_MAX_LENGTH 32

/// Size in number characters of SCPI parser input buffer.
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
#define SCPI_PARSER_INPUT_BUFFER_LENGTH 48
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
#define SCPI_PARSER_INPUT_BUFFER_LENGTH 2048
#endif

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
#define DP_OFF_DELAY_PERIOD 1

/// Text returned by the SYStem:CAPability command
#define STR_SYST_CAP "DCSUPPLY WITH (MEASURE|MULTIPLE|TRIGGER)"

/// Select type of TFT touch display, possible values are: TFT_320QVT_1289 and TFT_320QVT_9341
#define DISPLAY_TYPE TFT_320QVT_9341

/// Select display orientations, possible values are:
/// DISPLAY_ORIENTATION_PORTRAIT and DISPLAY_ORIENTATION_LANDSCAPE
#define DISPLAY_ORIENTATION_PORTRAIT 0
#define DISPLAY_ORIENTATION_LANDSCAPE 1
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
#define DISPLAY_ORIENTATION DISPLAY_ORIENTATION_PORTRAIT
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
#define DISPLAY_ORIENTATION DISPLAY_ORIENTATION_LANDSCAPE
#endif

/// Set to 1 to skip the test of PWRGOOD signal
#define CONF_SKIP_PWRGOOD_TEST 0

/// Minimal temperature (in oC) for sensor to be declared as valid.
#define TEMP_SENSOR_MIN_VALID_TEMPERATURE -5

/// Interval at which fan speed should be adjusted
#define FAN_SPEED_ADJUSTMENT_INTERVAL 10000

/// Interval at which fan speed should be measured
#define FAN_SPEED_MEASURMENT_INTERVAL 5000

/// Fan switch-on temperature (in oC)
#define FAN_MIN_TEMP 55

/// Max. allowed temperature (in oC), if it stays more then FAN_MAX_TEMP_DELAY seconds then main power will be turned off.
#define FAN_MAX_TEMP 75

///  PWM value for min. fan speed (12) 
#define FAN_MIN_PWM 15

/// PWM value for max. fan speed (255)
#define FAN_MAX_PWM 255

/// Max. allowed output current (in ampers) if fan or temp. sensor is invalid.
#define ERR_MAX_CURRENT 2.0f

/// Nominal fan RPM (for PWM=255).
#define FAN_NOMINAL_RPM 4500 

/// Number of seconds after which main power will be turned off.
#define FAN_MAX_TEMP_DELAY 30

/// Temperature drop (in oC) below FAN_MAX_TEMP to turn again main power on. Premature attempt to turn power on will report error -200.
#define FAN_MAX_TEMP_DROP 15 

/// Enable/disable RPM measurement during work - it will still be enabled at the boot during fan test.
#define FAN_OPTION_RPM_MEASUREMENT 1

/// Interval (in milliseconds) at which watchdog impulse will be sent
#define WATCHDOG_INTERVAL 250

/// Interval (in minutes) at which "on time" will be written to EEPROM
#define WRITE_ONTIME_INTERVAL 10

/// Maximum allowed length (including label) of the keypad text.
#define MAX_KEYPAD_TEXT_LENGTH 128

/// Frequency of master sync
#define SYNC_MASTER_FREQUENCY 320000 // 320kHz 

/// Enable transition to the Main page after period of inactivity.
#define GUI_BACK_TO_MAIN_ENABLED 1

/// Inactivity period duration in seconds before transition to the Main page.
#define GUI_BACK_TO_MAIN_DELAY 10

/// How much to wait (in seconds) for a lease for an IP address from a DHCP server
/// until we declare ethernet initialization failure.
#define ETHERNET_DHCP_TIMEOUT 15

/// Output power is monitored and if its go below DP_NEG_LEV
/// that is negative value in Watts (default -1 W),
/// and that condition lasts more then DP_NEG_DELAY seconds (default 5 s),
/// down-programmer circuit has to be switched off.
#define DP_NEG_LEV -1 // -1 W

/// See DP_NEG_LEV.
#define DP_NEG_DELAY 5 // 5 s

/// Replace standard SPI transactions implementation with in-house implementation,
/// It is more simple version where all interrupts are disabled during SPI transactions.
/// We had some problems (WATCHDOG, ADC timeout and EEPROM errros) with SPI in the
/// past and when we used our SPI transactions implementation, problems disappeared.
/// But, unfortunately, now ethernet doesn't work.
#define REPLACE_SPI_TRANSACTIONS_IMPLEMENTATION 0

/// Number of history values shown in YT diagram. This value must be the same as
/// the width of YT widget.
#define CHANNEL_HISTORY_SIZE 140

#define GUI_YT_VIEW_RATE_DEFAULT 0.1f
#define GUI_YT_VIEW_RATE_MIN 0.01f
#define GUI_YT_VIEW_RATE_MAX 300.0f

#define MAX_LIST_LENGTH 256

#define LIST_DWELL_MIN 0.0001f 
#define LIST_DWELL_MAX 65535.0f
#define LIST_DWELL_DEF 0.01f

#define MAX_LIST_COUNT 65535

#define PATH_SEPARATOR "/"
#define LISTS_DIR PATH_SEPARATOR "LISTS"
#define PROFILES_DIR PATH_SEPARATOR "PROFILES"
#define LIST_FILE_EXTENSION ".CSV"
#define MAX_PATH_LENGTH 128
#define CSV_SEPARATOR ','
#define LIST_CSV_FILE_NO_VALUE_CHAR '='

/// Time in seconds of SCPI inactivity to declare SCPI to be idle.
#define SCPI_IDLE_TIMEOUT 30

/// Changed but not confirmed value will be reset to current one
/// after this timeout in seconds.
/// See https://github.com/eez-open/psu-firmware/issues/84
#define ENCODER_CHANGE_TIMEOUT 15

#define DISPLAY_BRIGHTNESS_MIN 1
#define DISPLAY_BRIGHTNESS_MAX 20
#define DISPLAY_BRIGHTNESS_DEFAULT 20
