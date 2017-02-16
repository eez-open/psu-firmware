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

#define CH_BOARD_REVISION_R4B43A 0
#define CH_BOARD_REVISION_R5B6B  1
#define CH_BOARD_REVISION_R5B9   2
#define CH_BOARD_REVISION_R5B10  3

#define IOEXP_IODIR_R4B43A 0B01111101 // pins 1 and 7 set as output
#define IOEXP_IODIR_R5B6B  0B01100101 // pins 1, 3, 4 and 7 set as output
#define IOEXP_IODIR_R5B9   0B01100101 // pins 1, 3, 4 and 7 set as output
#define IOEXP_IODIR_R5B10  0B01100101 // pins 1, 3, 4 and 7 set as output

#define IOEXP_GPIO_INIT_R4B43A 0B00000010
#define IOEXP_GPIO_INIT_R5B6B  0B00001010
#define IOEXP_GPIO_INIT_R5B9   0B00001010
#define IOEXP_GPIO_INIT_R5B10  0B00001010

#define VOLTAGE_GND_OFFSET_R4B43A 0.0f
#define VOLTAGE_GND_OFFSET_R5B6B  0.86f
#define VOLTAGE_GND_OFFSET_R5B9   0.0f
#define VOLTAGE_GND_OFFSET_R5B10  0.86f

#define CURRENT_GND_OFFSET_R4B43A 0.0f
#define CURRENT_GND_OFFSET_R5B6B  0.110f
#define CURRENT_GND_OFFSET_R5B9   0.0f
#define CURRENT_GND_OFFSET_R5B10  0.110f

#define IO_BIT_NOT_USED 0

#define IO_BIT_OUT_SET_100_PERCENT_R5B6B 3
#define IO_BIT_OUT_EXT_PROG_R5B6B 4

#define IO_BIT_OUT_SET_100_PERCENT_R5B9 3
#define IO_BIT_OUT_EXT_PROG_R5B9 4

#define IO_BIT_OUT_SET_100_PERCENT_R5B10 3
#define IO_BIT_OUT_EXT_PROG_R5B10 4

#define CH_BOARD_REVISION_R4B43A_PARAMS CH_BOARD_REVISION_R4B43A, IOEXP_IODIR_R4B43A, IOEXP_GPIO_INIT_R4B43A, IO_BIT_NOT_USED,                  IO_BIT_NOT_USED,           VOLTAGE_GND_OFFSET_R4B43A, CURRENT_GND_OFFSET_R4B43A
#define CH_BOARD_REVISION_R5B6B_PARAMS  CH_BOARD_REVISION_R5B6B,  IOEXP_IODIR_R5B6B,  IOEXP_GPIO_INIT_R5B6B,  IO_BIT_OUT_SET_100_PERCENT_R5B6B, IO_BIT_OUT_EXT_PROG_R5B6B, VOLTAGE_GND_OFFSET_R5B6B,  CURRENT_GND_OFFSET_R5B6B
#define CH_BOARD_REVISION_R5B9_PARAMS   CH_BOARD_REVISION_R5B9,   IOEXP_IODIR_R5B9,   IOEXP_GPIO_INIT_R5B9,   IO_BIT_OUT_SET_100_PERCENT_R5B9,  IO_BIT_OUT_EXT_PROG_R5B9,  VOLTAGE_GND_OFFSET_R5B9,   CURRENT_GND_OFFSET_R5B9
#define CH_BOARD_REVISION_R5B10_PARAMS  CH_BOARD_REVISION_R5B10,  IOEXP_IODIR_R5B10,  IOEXP_GPIO_INIT_R5B10,  IO_BIT_OUT_SET_100_PERCENT_R5B10, IO_BIT_OUT_EXT_PROG_R5B10, VOLTAGE_GND_OFFSET_R5B10,  CURRENT_GND_OFFSET_R5B10

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9

#define CH_PINS_1 \
    ISOLATOR1_EN, IO_EXPANDER1, CONVEND1, ADC1_SELECT, DAC1_SELECT, \
    BP_LED_OUT1_PLUS, BP_LED_OUT1_MINUS, BP_LED_SENSE1_PLUS, BP_LED_SENSE1_MINUS, BP_RELAY_SENSE1, \
    LED_CC1, LED_CV1

#define CH_PINS_2 \
    ISOLATOR2_EN, IO_EXPANDER2, CONVEND2, ADC2_SELECT, DAC2_SELECT, \
    BP_LED_OUT2_PLUS, BP_LED_OUT2_MINUS, BP_LED_SENSE2_PLUS, BP_LED_SENSE2_MINUS, BP_RELAY_SENSE2, \
    LED_CC2, LED_CV2

#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12

#define CH_PINS_1 \
    ISOLATOR1_EN, IO_EXPANDER1, CONVEND1, ADC1_SELECT, DAC1_SELECT, \
    BP_LED_OUT1, BP_LED_SENSE1, BP_RELAY_SENSE1, \
    BP_LED_PROG1, BP_LED_CC1, BP_LED_CV1

#define CH_PINS_2 \
    ISOLATOR2_EN, IO_EXPANDER2, CONVEND2, ADC2_SELECT, DAC2_SELECT, \
    BP_LED_OUT2, BP_LED_SENSE2, BP_RELAY_SENSE2, \
    BP_LED_PROG2, BP_LED_CC2, BP_LED_CV2

#endif

//                       OVP_DEFAULT_STATE, OVP_MIN_DELAY, OVP_DEFAULT_DELAY, OVP_MAX_DELAY
#define CH_PARAMS_OVP    false,             0.0f,          0.005f,            10.0f

//                         U_MIN, U_DEF, U_MAX, U_MAX_CONF, U_MIN_STEP, U_DEF_STEP, U_MAX_STEP, U_CAL_VAL_MIN, U_CAL_VAL_MID, U_CAL_VAL_MAX, U_CURR_CAL
#define CH_PARAMS_U_30V    0.0f,  0.0f,  30.0f, 30.0f,      0.01f,      0.1f,       5.0f,       0.15f,         14.1f,         28.0f,         25.0f,     CH_PARAMS_OVP
#define CH_PARAMS_U_40V    0.0f,  0.0f,  40.0f, 40.0f,      0.01f,      0.1f,       5.0f,       0.15f,         19.1f,         38.0f,         25.0f,     CH_PARAMS_OVP
#define CH_PARAMS_U_50V    0.0f,  0.0f,  50.0f, 50.0f,      0.01f,      0.1f,       5.0f,       0.15f,         24.1f,         48.0f,         25.0f,     CH_PARAMS_OVP

//                       OCP_DEFAULT_STATE, OCP_MIN_DELAY, OCP_DEFAULT_DELAY, OCP_MAX_DELAY
#define CH_PARAMS_OCP    false,             0.0f,          0.02f,             10.0f

//                        I_MIN, I_DEF, I_MAX,  I_MIN_STEP, I_DEF_STEP, I_MAX_STEP, I_CAL_VAL_MIN, I_CAL_VAL_MID, I_CAL_VAL_MAX, I_VOLT_CAL
#define CH_PARAMS_I_3A    0.0f,  0.0f,  3.125f, 0.01f,      0.01f,      1.0f,       0.05f,         1.525f,        3.0f,          0.05f,      CH_PARAMS_OCP
#define CH_PARAMS_I_5A    0.0f,  0.0f,  5.0f,   0.01f,      0.01f,      1.0f,       0.05f,         2.425f,        4.8f,          0.05f,      CH_PARAMS_OCP

//                             OPP_MIN_DELAY, OPP_DEFAULT_DELAY, OPP_MAX_DELAY
#define CH_PARAMS_OPP_DELAY    1.0f,          10.0f,             300.0f

// Channel's OPP, max. power and post-regulator SOA
//
//                                                                    OPP_DEFAULT_STATE          OPP_MIN_LEVEL          SOA_VIN 
//                                                                    |                          | OPP_DEFAULT_LEVEL    |      SOA_PREG_CURR,
//                                                                    |                          |      |  OPP_MAX_LEVEL|      |       SOA_POSTREG_PTOT
//                                                                    |                          |      |       |       |      |       |      PTOT      
#define CH_PARAMS_30V_3A             CH_PARAMS_U_30V, CH_PARAMS_I_3A, true, CH_PARAMS_OPP_DELAY,  0.0f,  60.0f,  90.0f, 38.0f, 3.125f, 25.0f, 90.0f
#define CH_PARAMS_40V_3A             CH_PARAMS_U_40V, CH_PARAMS_I_3A, true, CH_PARAMS_OPP_DELAY,  0.0f,  80.0f, 120.0f, 48.0f, 3.125f, 25.0f, 120.0f
#define CH_PARAMS_50V_3A             CH_PARAMS_U_50V, CH_PARAMS_I_3A, true, CH_PARAMS_OPP_DELAY,  0.0f, 100.0f, 150.0f, 58.0f, 3.125f, 25.0f, 150.0f
#define CH_PARAMS_30V_5A             CH_PARAMS_U_30V, CH_PARAMS_I_5A, true, CH_PARAMS_OPP_DELAY,  0.0f, 100.0f, 120.0f, 38.0f,   5.0f, 25.0f, 120.0f
#define CH_PARAMS_40V_5A             CH_PARAMS_U_40V, CH_PARAMS_I_5A, true, CH_PARAMS_OPP_DELAY,  0.0f, 155.0f, 155.0f, 48.0f,   5.0f, 25.0f, 155.0f
#define CH_PARAMS_50V_5A             CH_PARAMS_U_50V, CH_PARAMS_I_5A, true, CH_PARAMS_OPP_DELAY,  0.0f, 160.0f, 200.0f, 58.0f,   5.0f, 25.0f, 200.0f

