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

#define CH_BOARD_REVISION_R4B43A 0
#define CH_BOARD_REVISION_R5B6B  1

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9

#define CH_PINS_1 \
    ISOLATOR1_EN, IO_EXPANDER1, CONVEND1, ADC1_SELECT, DAC1_SELECT, \
    BP_LED_OUT1_PLUS, BP_LED_OUT1_MINUS, BP_LED_SENSE1_PLUS, BP_LED_SENSE1_MINUS, BP_RELAY_SENSE1, \
    LED_CC1, LED_CV1

#define CH_PINS_2 \
    ISOLATOR2_EN, IO_EXPANDER2, CONVEND2, ADC2_SELECT, DAC2_SELECT, \
    BP_LED_OUT2_PLUS, BP_LED_OUT2_MINUS, BP_LED_SENSE2_PLUS, BP_LED_SENSE2_MINUS, BP_RELAY_SENSE2, \
    LED_CC2, LED_CV2

#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6

#define CH_PINS_1 \
    ISOLATOR1_EN, IO_EXPANDER1, CONVEND1, ADC1_SELECT, DAC1_SELECT, \
    BP_LED_OUT1, BP_LED_SENSE1, BP_RELAY_SENSE1, \
    BP_LED_CC1, BP_LED_CV1

#define CH_PINS_2 \
    ISOLATOR2_EN, IO_EXPANDER2, CONVEND2, ADC2_SELECT, DAC2_SELECT, \
    BP_LED_OUT2, BP_LED_SENSE2, BP_RELAY_SENSE2, \
    BP_LED_CC2, BP_LED_CV2

#endif

#define CH_PARAMS_OVP    false, 0.0f, 0.005f, 10.0f

#define CH_PARAMS_U_30V    0.0f, 0.0f, 30.0f, 0.01f, 0.1f, 5.0f, 0.2f, 14.1f, 28.0f, 25.0f, CH_PARAMS_OVP
#define CH_PARAMS_U_40V    0.0f, 0.0f, 40.0f, 0.01f, 0.1f, 5.0f, 0.2f, 19.1f, 38.0f, 25.0f, CH_PARAMS_OVP
#define CH_PARAMS_U_50V    0.0f, 0.0f, 50.0f, 0.01f, 0.1f, 5.0f, 0.2f, 24.1f, 48.0f, 25.0f, CH_PARAMS_OVP

#define CH_PARAMS_OCP    false, 0.0f, 0.02f, 10.0f

#define CH_PARAMS_I_3A    0.0f, 0.0f, 3.125f, 0.01f, 0.01f, 1.0f, 0.05f, 1.525f, 3.0f, 0.05f, CH_PARAMS_OCP
#define CH_PARAMS_I_5A    0.0f, 0.0f, 5.0f,   0.01f, 0.01f, 1.0f, 0.05f, 2.425f, 4.8f, 0.05f, CH_PARAMS_OCP

#define CH_PARAMS_OPP_DELAY    1.0f, 10.0f, 300.0f

#define CH_PARAMS_30V_3A    CH_PARAMS_U_30V, CH_PARAMS_I_3A, true, CH_PARAMS_OPP_DELAY, 10.0f,  60.0f,  90.0f
#define CH_PARAMS_40V_3A    CH_PARAMS_U_40V, CH_PARAMS_I_3A, true, CH_PARAMS_OPP_DELAY, 10.0f,  80.0f, 120.0f
#define CH_PARAMS_50V_3A    CH_PARAMS_U_50V, CH_PARAMS_I_3A, true, CH_PARAMS_OPP_DELAY, 10.0f, 100.0f, 150.0f
#define CH_PARAMS_30V_5A    CH_PARAMS_U_30V, CH_PARAMS_I_5A, true, CH_PARAMS_OPP_DELAY, 10.0f, 100.0f, 120.0f
#define CH_PARAMS_40V_5A    CH_PARAMS_U_40V, CH_PARAMS_I_5A, true, CH_PARAMS_OPP_DELAY, 10.0f, 150.0f, 160.0f
#define CH_PARAMS_50V_5A    CH_PARAMS_U_50V, CH_PARAMS_I_5A, true, CH_PARAMS_OPP_DELAY, 10.0f, 150.0f, 160.0f
