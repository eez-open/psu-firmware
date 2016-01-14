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
 
#pragma once

// Channel's pin usage as defined in eez_psu.h
#define CH_PINS_1 \
    ISOLATOR1_EN, IO_EXPANDER1, CONVEND1, ADC1_SELECT, DAC1_SELECT, \
    BP_LED_OUT1_PLUS, BP_LED_OUT1_MINUS, BP_LED_SENSE1_PLUS, BP_LED_SENSE1_MINUS, BP_RELAY_SENSE1, \
    LED_CC1, LED_CV1 \

#define CH_PINS_2 \
    ISOLATOR2_EN, IO_EXPANDER2, CONVEND2, ADC2_SELECT, DAC2_SELECT, \
    BP_LED_OUT2_PLUS, BP_LED_OUT2_MINUS, BP_LED_SENSE2_PLUS, BP_LED_SENSE2_MINUS, BP_RELAY_SENSE2, \
    LED_CC2, LED_CV2 \

// Over-voltage protection parameters.
//
// OVP_DEFAULT_STATE - default OVP state
// OVP_MIN_DELAY - OVP MINimum constant value in seconds
// OVP_DEFAULT_DELAY - OVP DEFault constant value in seconds 
// OVP_MAX_DELAY - OVP MAXimum constant value in seconds
#define CH_PARAMS_OVP    false, 0.0f, 0.005f, 10.0f

// Channel output voltage programming parameters.
// Three ranges are defined: 0 - 30 V, 0 - 40 V and 0 - 50 V.
// Please note that for other ranges the post-regulator's
// CV control loop gain has to be adjusted.
// 
// U_MIN - MINimum constant value in volts
// U_DEF - DEFault constant value in volts
// U_MAX - MAXimum constant value in volts
// U_MIN_STEP - MINimum voltage step constant value in volts
// U_DEF_STEP - DEFault voltage step constant value in volts
// U_MAX_STEP - MAXimum voltage step constant value in volts
// U_CAL_VAL_MIN - Programmed output voltage in volts when MINimum LEVel in calibration state is selected 
// U_CAL_VAL_MID - Programmed output voltage in volts when MIDdle LEVel in calibration state is selected 
// U_CAL_VAL_MAX - Programmed output voltage in volts when MAXimum LEVel in calibration state is selected   
// U_CURR_CAL - Programmed output voltage in volts during calibration of current
// CH_PARAMS_OVP - Channel's OVP parameters
#define CH_PARAMS_U_30V    0.0f, 0.0f, 30.0f, 0.01f, 0.1f, 5.0f, 0.2f, 14.1f, 28.0f, 25.0f, CH_PARAMS_OVP
#define CH_PARAMS_U_40V    0.0f, 0.0f, 40.0f, 0.01f, 0.1f, 5.0f, 0.2f, 19.1f, 38.0f, 25.0f, CH_PARAMS_OVP
#define CH_PARAMS_U_50V    0.0f, 0.0f, 50.0f, 0.01f, 0.1f, 5.0f, 0.2f, 24.1f, 48.0f, 25.0f, CH_PARAMS_OVP

// Over-current protection parameters.
//
// OCP_DEFAULT_STATE - default OCP state
// OCP_MIN_DELAY - OCP MINimum constant value in seconds
// OCP_DEFAULT_DELAY - OCP DEFault constant value in seconds 
// OCP_MAX_DELAY - OCP MAXimum constant value in seconds
#define CH_PARAMS_OCP    false, 0.0f, 0.02f, 10.0f

// Channel output current programming parameters.
// Two ranges are defined: 0 - 3.12 A and 0 - 5 A.
// Please note that for other ranges the post-regulator's 
// CC control loop gain and/or current sense resistor value 
// has to be adjusted.
//
// I_MIN - MINimum constant value in amperes
// I_DEF - DEFault constant value in amperes
// I_MAX - MAXimum constant value in amperes
// I_MIN_STEP - MINimum current step constant value in amperes
// I_DEF_STEP - DEFault current step constant value in amperes
// I_MAX_STEP - MAXimum current step constant value in amperes
// I_CAL_VAL_MIN - Programmed output current in amperes when MINimum LEVel in calibration state is selected
// I_CAL_VAL_MID - Programmed output current in amperes when MIDdle LEVel in calibration state is selected
// I_CAL_VAL_MAX - Programmed output current in amperes when MAXimum LEVel in calibration state is selected
// I_VOLT_CAL - Programmed output current in amperes during calibration of voltage (has to be greater then 0 A!)
// CH_PARAMS_OCP - Channel's OCP parameters
#define CH_PARAMS_I_3A    0.0f, 0.0f, 3.125f, 0.01f, 0.01f, 1.0f, 0.05f, 1.525f, 3.0f, 0.05f, CH_PARAMS_OCP
#define CH_PARAMS_I_5A    0.0f, 0.0f, 5.0f,   0.01f, 0.01f, 1.0f, 0.05f, 2.425f, 4.8f, 0.05f, CH_PARAMS_OCP

// General over-power protection parameters.
//
// OPP_MIN_DELAY - OPP MINimum constant value in watts
// OPP_DEFAULT_DELAY - OPP DEFault constant value in watts
// OPP_MAX_DELAY - OPP MAXimum constant value in watts
#define CH_PARAMS_OPP_DELAY    1.0f, 10.0f, 300.0f

// Channel over-power protection parameters.
//
// CH_PARAMS_U_XX - Channel output voltage programming parameters
// CH_PARAMS_I_XX - Channel output current programming parameters
// OPP_DEFAULT_STATE - default OPP state
// CH_PARAMS_OPP_DELAY - General OPP parameters
// OPP_MIN_LEVEL - OPP MINimum LEVel constant value in watts 
// OPP_DEFAULT_LEVEL - OPP DEFault LEVel constant value in watts 
// OPP_MAX_LEVEL - OPP MAXimum LEVel constant value in watts 
#define CH_PARAMS_30V_3A    CH_PARAMS_U_30V, CH_PARAMS_I_3A, true, CH_PARAMS_OPP_DELAY, 10.0f,  60.0f,  90.0f
#define CH_PARAMS_40V_3A    CH_PARAMS_U_40V, CH_PARAMS_I_3A, true, CH_PARAMS_OPP_DELAY, 10.0f,  80.0f, 120.0f
#define CH_PARAMS_50V_3A    CH_PARAMS_U_50V, CH_PARAMS_I_3A, true, CH_PARAMS_OPP_DELAY, 10.0f, 100.0f, 150.0f
#define CH_PARAMS_30V_5A    CH_PARAMS_U_30V, CH_PARAMS_I_5A, true, CH_PARAMS_OPP_DELAY, 10.0f, 100.0f, 120.0f
#define CH_PARAMS_40V_5A    CH_PARAMS_U_40V, CH_PARAMS_I_5A, true, CH_PARAMS_OPP_DELAY, 10.0f, 150.0f, 160.0f
#define CH_PARAMS_50V_5A    CH_PARAMS_U_50V, CH_PARAMS_I_5A, true, CH_PARAMS_OPP_DELAY, 10.0f, 150.0f, 160.0f
