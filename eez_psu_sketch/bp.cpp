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
 
#include "psu.h"
#include "bp.h"

namespace eez {
namespace psu {
namespace bp {

psu::TestResult test_result = psu::TEST_SKIPPED;

static uint16_t g_lastConf;
static int g_channelCouplingType;

////////////////////////////////////////////////////////////////////////////////

void set(uint16_t conf) {
    if (OPTION_BP) {
        SPI.beginTransaction(TLC5925_SPI);
        digitalWrite(BP_OE, HIGH);
        digitalWrite(BP_SELECT, LOW);
        SPI.transfer(conf >> 8);
        SPI.transfer(conf & 0xFF);
        g_lastConf = conf;
        digitalWrite(BP_SELECT, HIGH);
        digitalWrite(BP_SELECT, LOW);
        digitalWrite(BP_OE, LOW);
        SPI.endTransaction();

        DebugTraceF("BP 0x%04x", (int)conf);
    }
}

void bp_switch(uint16_t mask, bool on) {
    uint16_t conf = g_lastConf;

    if (on) {
        conf |= mask;
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
        if (mask & (1 << BP_LED_OUT1_PLUS)) conf &= ~(1 << BP_LED_OUT1_PLUS_RED);
        if (mask & (1 << BP_LED_OUT1_MINUS)) conf &= ~(1 << BP_LED_OUT1_MINUS_RED);
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
        if (mask & (1 << BP_LED_OUT1)) conf &= ~(1 << BP_LED_OUT1_RED);
        if (mask & (1 << BP_LED_OUT1_RED)) conf &= ~((1 << BP_LED_OUT1) | (1 << BP_LED_OUT2));
#endif
    }
    else {
        conf &= ~mask;
    }

    if (conf != g_lastConf) {
        set(conf);
    }
}

void bp_switch2(uint16_t maskOn, uint16_t maskOff) {
    uint16_t conf = g_lastConf;

    conf |= maskOn;
    conf &= ~maskOff;

    if (conf != g_lastConf) {
        set(conf);
    }
}

////////////////////////////////////////////////////////////////////////////////

void init() {
    switchStandby(true);
}

void switchStandby(bool on) {
    set(on ? (1 << BP_STANDBY) : 0);
}

void switchOutput(Channel *channel, bool on) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    if (g_channelCouplingType == CHANNELS_COUPLING_TYPE_NONE) {
        bp_switch((1 << channel->bp_led_out_plus) | (1 << channel->bp_led_out_minus), on);
    } else {
        if (channe->index == 1) {
            bp_switch((1 << BP_LED_OUT1_PLUS_RED) | (1 << BP_LED_OUT1_MINUS_RED), on);
        }
    }
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
    if (g_channelCouplingType == CHANNELS_COUPLING_TYPE_NONE) {
        bp_switch((1 << channel->bp_led_out), on);
    } else {
        bp_switch((1 << BP_LED_OUT1_RED), on);
    }
#endif
}

void switchSense(Channel *channel, bool on) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    bp_switch((1 << channel->bp_led_sense_plus) |
        (1 << channel->bp_led_sense_minus) |
        (1 << channel->bp_relay_sense), on);
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
    bp_switch((1 << channel->bp_led_sense) | (1 << channel->bp_relay_sense), on);
#endif
}

void switchProg(Channel *channel, bool on) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4    
    bp_switch(1 << channel->bp_led_prog, on);
#endif
}

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4

void cvLedSwitch(Channel *channel, bool on) {
    bp_switch(1 << channel->cv_led_pin, on);
}

void ccLedSwitch(Channel *channel, bool on) {
    bp_switch(1 << channel->cc_led_pin, on);
}

#endif

void switchChannelCoupling(int channelCouplingType) {
    g_channelCouplingType = channelCouplingType;

    if (g_channelCouplingType == CHANNELS_COUPLING_TYPE_PARALLEL) {
        bp_switch2(1 << BP_K_PAR, 1 << BP_K_SER);
    } else if (g_channelCouplingType == CHANNELS_COUPLING_TYPE_SERIES) {
        bp_switch2(1 << BP_K_SER, 1 << BP_K_PAR);
    } else {
        bp_switch2(0, (1 << BP_K_SER) | (1 << BP_K_PAR));
    }
}

}
}
} // namespace eez::psu::bp