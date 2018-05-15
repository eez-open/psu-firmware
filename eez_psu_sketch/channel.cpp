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
#include "serial_psu.h"

#if OPTION_ETHERNET
#include "ethernet.h"
#endif
#if (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12) && OPTION_WATCHDOG
#include "watchdog.h"
#endif

#include "bp.h"
#include "board.h"
#include "ioexp.h"
#include "adc.h"
#include "dac.h"
#include "bp.h"
#include "calibration.h"
#include "persist_conf.h"
#include "sound.h"
#include "profile.h"
#include "event_queue.h"
#include "channel_dispatcher.h"
#include "list_program.h"
#include "trigger.h"
#include "io_pins.h"

namespace eez {
namespace psu {

using namespace scpi;

////////////////////////////////////////////////////////////////////////////////

const char *CH_BOARD_REVISION_NAMES[] = {
    // CH_BOARD_REVISION_R4B43A
    "Power_r4b43a",
    // CH_BOARD_REVISION_R5B6B
    "Power_r5B6b",
    // CH_BOARD_REVISION_R5B9
    "Power_r5B9",
    // CH_BOARD_REVISION_R5B10
    "Power_r5B10",
    // CH_BOARD_REVISION_R5B12
    "Power_r5B12"
};

uint16_t CH_BOARD_REVISION_FEATURES[] = {
    // CH_BOARD_REVISION_R4B43A
    CH_FEATURE_VOLT | CH_FEATURE_CURRENT | CH_FEATURE_OE,
    // CH_BOARD_REVISION_R5B6B
    CH_FEATURE_VOLT | CH_FEATURE_CURRENT | CH_FEATURE_POWER | CH_FEATURE_OE | CH_FEATURE_DPROG | CH_FEATURE_LRIPPLE | CH_FEATURE_RPROG,
    // CH_BOARD_REVISION_R5B9
    CH_FEATURE_VOLT | CH_FEATURE_CURRENT | CH_FEATURE_POWER | CH_FEATURE_OE | CH_FEATURE_DPROG | CH_FEATURE_LRIPPLE | CH_FEATURE_RPROG | CH_FEATURE_RPOL,
    // CH_BOARD_REVISION_R5B10
    CH_FEATURE_VOLT | CH_FEATURE_CURRENT | CH_FEATURE_POWER | CH_FEATURE_OE | CH_FEATURE_DPROG | CH_FEATURE_LRIPPLE | CH_FEATURE_RPROG | CH_FEATURE_RPOL,
    // CH_BOARD_REVISION_R5B12
    CH_FEATURE_VOLT | CH_FEATURE_CURRENT | CH_FEATURE_POWER | CH_FEATURE_OE | CH_FEATURE_DPROG | CH_FEATURE_LRIPPLE | CH_FEATURE_RPROG | CH_FEATURE_RPOL
};

////////////////////////////////////////////////////////////////////////////////

#define CHANNEL(INDEX, BOARD_REVISION, PINS, PARAMS) Channel(INDEX, BOARD_REVISION, PINS, PARAMS)
Channel channels[CH_MAX] = { CHANNELS };
#undef CHANNEL

////////////////////////////////////////////////////////////////////////////////

Channel &Channel::get(int channel_index) {
    return channels[channel_index];
}

////////////////////////////////////////////////////////////////////////////////

void Channel::Value::init(float set_, float step_, float limit_) {
    set = set_;
    step = step_;
    limit = limit_;
    resetMonValues();
}

void Channel::Value::resetMonValues() {
    mon_adc = 0;
    mon = 0;
    mon_last = 0;
    mon_dac = 0;

    mon_index = -1;
    mon_dac_index = -1;

    mon_measured = false;
}

void Channel::Value::addMonValue(float value) {
    mon_last = value;

    if (mon_index == -1) {
        mon_index = 0;
        for (int i = 0; i < NUM_ADC_AVERAGING_VALUES; ++i) {
            mon_arr[i] = value;
        }
        mon_total = NUM_ADC_AVERAGING_VALUES * value;
        mon = value;
    } else {
        mon_total -= mon_arr[mon_index];
        mon_total += value;
        mon_arr[mon_index] = value;
        mon_index = (mon_index + 1) % NUM_ADC_AVERAGING_VALUES;
        mon = mon_total / NUM_ADC_AVERAGING_VALUES;
    }

    mon_measured = true;
}

void Channel::Value::addMonDacValue(float value) {
    if (mon_dac_index == -1) {
        mon_dac_index = 0;
        for (int i = 0; i < NUM_ADC_AVERAGING_VALUES; ++i) {
            mon_dac_arr[i] = value;
        }
        mon_dac_total = NUM_ADC_AVERAGING_VALUES * value;
        mon_dac = value;
    } else {
        mon_dac_total -= mon_dac_arr[mon_dac_index];
        mon_dac_total += value;
        mon_dac_arr[mon_dac_index] = value;
        mon_dac_index = (mon_dac_index + 1) % NUM_ADC_AVERAGING_VALUES;
        mon_dac = mon_dac_total / NUM_ADC_AVERAGING_VALUES;
    }
}

////////////////////////////////////////////////////////////////////////////////

static struct {
    unsigned OE_SAVED: 1;
    unsigned CH1_OE: 1;
    unsigned CH2_OE: 1;
} g_savedOE;

void Channel::saveAndDisableOE() {
    if (!g_savedOE.OE_SAVED) {
        if (CH_NUM > 0) {
            g_savedOE.CH1_OE = Channel::get(0).isOutputEnabled() ? 1 : 0;
            Channel::get(0).outputEnable(false);

            if (CH_NUM > 1) {
                g_savedOE.CH2_OE = Channel::get(1).isOutputEnabled() ? 1 : 0;
                Channel::get(1).outputEnable(false);
            }
        }
        g_savedOE.OE_SAVED = 1;
    }
}

void Channel::restoreOE() {
    if (g_savedOE.OE_SAVED) {
        if (CH_NUM > 0) {
            Channel::get(0).outputEnable(g_savedOE.CH1_OE ? true : false);
            if (CH_NUM > 1) {
                Channel::get(1).outputEnable(g_savedOE.CH2_OE ? true : false);
            }
        }
        g_savedOE.OE_SAVED = 0;
    }
}

char *Channel::getChannelsInfo(char *p) {
    bool ch_used[CH_MAX];

    for (int i = 0; i < CH_NUM; ++i) {
        ch_used[i] = false;
    }

    bool first_channel = true;

    for (int i = 0; i < CH_NUM; ++i) {
        if (!ch_used[i]) {
            int count = 1;
            for (int j = i + 1; j < CH_NUM; ++j) {
                if (Channel::get(i).U_MAX == Channel::get(j).U_MAX && Channel::get(i).I_MAX == Channel::get(j).I_MAX) {
                    ch_used[j] = true;
                    ++count;
                }
            }

            if (first_channel) {
                *p++ += ' ';
                first_channel = false;
            }
            else {
                *p++ += '-';
            }

            p += sprintf(p, "%d/%02d/%02d", count, (int)floor(Channel::get(i).U_MAX), (int)floor(Channel::get(i).I_MAX));
        }
    }

    return p;
}

char *Channel::getChannelsInfoShort(char *p) {
    bool ch_used[CH_MAX];

    for (int i = 0; i < CH_NUM; ++i) {
        ch_used[i] = false;
    }

    bool first_channel = true;

    for (int i = 0; i < CH_NUM; ++i) {
        if (!ch_used[i]) {
            int count = 1;
            for (int j = i + 1; j < CH_NUM; ++j) {
                if (Channel::get(i).U_MAX == Channel::get(j).U_MAX && Channel::get(i).I_MAX == Channel::get(j).I_MAX) {
                    ch_used[j] = true;
                    ++count;
                }
            }

            if (first_channel) {
                first_channel = false;
            }
            else {
                *p++ += ' ';
                *p++ += '-';
                *p++ += ' ';
            }

            p += sprintf(p, "%d V / %d A", (int)floor(Channel::get(i).U_MAX), (int)floor(Channel::get(i).I_MAX));
        }
    }

    return p;
}

////////////////////////////////////////////////////////////////////////////////

#ifdef EEZ_PLATFORM_SIMULATOR

void Channel::Simulator::setLoadEnabled(bool value) {
    load_enabled = value;
    profile::save();
}

bool Channel::Simulator::getLoadEnabled() {
    return load_enabled;
}

void Channel::Simulator::setLoad(float value) {
    load = value;
    profile::save();
}

float Channel::Simulator::getLoad() {
    return load;
}

void Channel::Simulator::setVoltProgExt(float value) {
    voltProgExt = value;
    profile::save();
}

float Channel::Simulator::getVoltProgExt() {
    return voltProgExt;
}

#endif

////////////////////////////////////////////////////////////////////////////////

Channel::Channel(
    uint8_t index_,
    uint8_t boardRevision_, uint8_t ioexp_iodir_, uint8_t ioexp_gpio_init_, uint8_t IO_BIT_OUT_SET_100_PERCENT_, uint8_t IO_BIT_OUT_EXT_PROG_, float VOLTAGE_GND_OFFSET_, float CURRENT_GND_OFFSET_,
    uint8_t isolator_pin_, uint8_t ioexp_pin_, uint8_t convend_pin_, uint8_t adc_pin_, uint8_t dac_pin_,
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    uint8_t bp_led_out_plus_, uint8_t bp_led_out_minus_, uint8_t bp_led_sense_plus_, uint8_t bp_led_sense_minus_, uint8_t bp_relay_sense_,
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
    uint8_t bp_led_out_, uint8_t bp_led_sense_, uint8_t bp_relay_sense_, uint8_t bp_led_prog_,
#endif
    uint8_t cc_led_pin_, uint8_t cv_led_pin_,
    float U_MIN_, float U_DEF_, float U_MAX_, float U_MAX_CONF_, float U_MIN_STEP_, float U_DEF_STEP_, float U_MAX_STEP_, float U_CAL_VAL_MIN_, float U_CAL_VAL_MID_, float U_CAL_VAL_MAX_, float U_CURR_CAL_,
    bool OVP_DEFAULT_STATE_, float OVP_MIN_DELAY_, float OVP_DEFAULT_DELAY_, float OVP_MAX_DELAY_,
    float I_MIN_, float I_DEF_, float I_MAX_, float I_MAX_CONF_, float I_MIN_STEP_, float I_DEF_STEP_, float I_MAX_STEP_, float I_CAL_VAL_MIN_, float I_CAL_VAL_MID_, float I_CAL_VAL_MAX_, float I_VOLT_CAL_,
    bool OCP_DEFAULT_STATE_, float OCP_MIN_DELAY_, float OCP_DEFAULT_DELAY_, float OCP_MAX_DELAY_,
    bool OPP_DEFAULT_STATE_, float OPP_MIN_DELAY_, float OPP_DEFAULT_DELAY_, float OPP_MAX_DELAY_, float OPP_MIN_LEVEL_, float OPP_DEFAULT_LEVEL_, float OPP_MAX_LEVEL_,
    float SOA_VIN_, float SOA_PREG_CURR_, float SOA_POSTREG_PTOT_, float PTOT_
    )

    :

	index(index_),
    boardRevision(boardRevision_), ioexp_iodir(ioexp_iodir_), ioexp_gpio_init(ioexp_gpio_init_),
    isolator_pin(isolator_pin_), ioexp_pin(ioexp_pin_), convend_pin(convend_pin_), adc_pin(adc_pin_), dac_pin(dac_pin_),
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    bp_led_out_plus(bp_led_out_plus_), bp_led_out_minus(bp_led_out_minus_), bp_led_sense_plus(bp_led_sense_plus_), bp_led_sense_minus(bp_led_sense_minus_), bp_relay_sense(bp_relay_sense_),
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
    bp_led_out(bp_led_out_), bp_led_sense(bp_led_sense_), bp_relay_sense(bp_relay_sense_), bp_led_prog(bp_led_prog_),
#endif
    cc_led_pin(cc_led_pin_), cv_led_pin(cv_led_pin_),
    U_MIN(U_MIN_), U_DEF(U_DEF_), U_MAX(U_MAX_), U_MAX_CONF(U_MAX_CONF_), U_MIN_STEP(U_MIN_STEP_), U_DEF_STEP(U_DEF_STEP_), U_MAX_STEP(U_MAX_STEP_), U_CAL_VAL_MIN(U_CAL_VAL_MIN_), U_CAL_VAL_MID(U_CAL_VAL_MID_), U_CAL_VAL_MAX(U_CAL_VAL_MAX_), U_CURR_CAL(U_CURR_CAL_),
    OVP_DEFAULT_STATE(OVP_DEFAULT_STATE_), OVP_MIN_DELAY(OVP_MIN_DELAY_), OVP_DEFAULT_DELAY(OVP_DEFAULT_DELAY_), OVP_MAX_DELAY(OVP_MAX_DELAY_),
    I_MIN(I_MIN_), I_DEF(I_DEF_), I_MAX(I_MAX_), I_MAX_CONF(I_MAX_CONF_), I_MIN_STEP(I_MIN_STEP_), I_DEF_STEP(I_DEF_STEP_), I_MAX_STEP(I_MAX_STEP_), I_CAL_VAL_MIN(I_CAL_VAL_MIN_), I_CAL_VAL_MID(I_CAL_VAL_MID_), I_CAL_VAL_MAX(I_CAL_VAL_MAX_), I_VOLT_CAL(I_VOLT_CAL_),
    OCP_DEFAULT_STATE(OCP_DEFAULT_STATE_), OCP_MIN_DELAY(OCP_MIN_DELAY_), OCP_DEFAULT_DELAY(OCP_DEFAULT_DELAY_), OCP_MAX_DELAY(OCP_MAX_DELAY_),
    OPP_DEFAULT_STATE(OPP_DEFAULT_STATE_), OPP_MIN_DELAY(OPP_MIN_DELAY_), OPP_DEFAULT_DELAY(OPP_DEFAULT_DELAY_), OPP_MAX_DELAY(OPP_MAX_DELAY_), OPP_MIN_LEVEL(OPP_MIN_LEVEL_), OPP_DEFAULT_LEVEL(OPP_DEFAULT_LEVEL_), OPP_MAX_LEVEL(OPP_MAX_LEVEL_),
    SOA_VIN(SOA_VIN_), SOA_PREG_CURR(SOA_PREG_CURR_), SOA_POSTREG_PTOT(SOA_POSTREG_PTOT_), PTOT(PTOT_),
    ioexp(*this, IO_BIT_OUT_SET_100_PERCENT_, IO_BIT_OUT_EXT_PROG_),
    adc(*this),
    dac(*this),
    onTimeCounter(index_),
    VOLTAGE_GND_OFFSET(VOLTAGE_GND_OFFSET_),
    CURRENT_GND_OFFSET(CURRENT_GND_OFFSET_)
{
    u.min = U_MIN;
    u.max = U_MAX;
    u.def = U_DEF;

    i.min = I_MIN;
    i.max = I_MAX;
    i.def = I_DEF;

    //negligibleAdcDiffForVoltage2 = (int)((AnalogDigitalConverter::ADC_MAX - AnalogDigitalConverter::ADC_MIN) / (2 * 100 * (U_MAX - U_MIN))) + 1;
    //negligibleAdcDiffForVoltage3 = (int)((AnalogDigitalConverter::ADC_MAX - AnalogDigitalConverter::ADC_MIN) / (2 * 1000 * (U_MAX - U_MIN))) + 1;
    //calculateNegligibleAdcDiffForCurrent();

#ifdef EEZ_PLATFORM_SIMULATOR
    simulator.load_enabled = true;
    simulator.load = 10;
#endif

    uBeforeBalancing = NAN;
    iBeforeBalancing = NAN;

    flags.currentCurrentRange = CURRENT_RANGE_HIGH;
    flags.currentRangeSelectionMode = CURRENT_RANGE_SELECTION_USE_BOTH;
    flags.autoSelectCurrentRange = 1;

    flags.displayValue1 = DISPLAY_VALUE_VOLTAGE;
    flags.displayValue2 = DISPLAY_VALUE_CURRENT;
    ytViewRate = GUI_YT_VIEW_RATE_DEFAULT;

    autoRangeCheckLastTickCount = 0;

    flags.cvMode = 0;
    flags.ccMode = 0;
    updateCcAndCvSwitch();
}

void Channel::protectionEnter(ProtectionValue &cpv) {
    channel_dispatcher::outputEnable(*this, false);

    cpv.flags.tripped = 1;

    int bit_mask = reg_get_ques_isum_bit_mask_for_channel_protection_value(this, cpv);
    setQuesBits(bit_mask, true);

    int16_t eventId = event_queue::EVENT_ERROR_CH1_OVP_TRIPPED + 3 * (index - 1);

    if (IS_OVP_VALUE(this, cpv)) {
        if (flags.rprogEnabled && mw::equal(channel_dispatcher::getUProtectionLevel(*this), channel_dispatcher::getUMax(*this), getPrecision(UNIT_VOLT))) {
            psu::g_rprogAlarm = true;
        }
        doRemoteProgrammingEnable(false);
    } else if (IS_OCP_VALUE(this, cpv)) {
        eventId += 1;
    } else {
        eventId += 2;
    }

    event_queue::pushEvent(eventId);

    onProtectionTripped();
}

void Channel::protectionCheck(ProtectionValue &cpv) {
    bool state;
    bool condition;
    float delay;

    if (IS_OVP_VALUE(this, cpv)) {
        state = flags.rprogEnabled || prot_conf.flags.u_state;
        //condition = flags.cv_mode && (!flags.cc_mode || fabs(i.mon_last - i.set) >= CHANNEL_VALUE_PRECISION) && (prot_conf.u_level <= u.set);
        condition = mw::greaterOrEqual(channel_dispatcher::getUMon(*this), channel_dispatcher::getUProtectionLevel(*this), getPrecision(UNIT_VOLT))
            || (flags.rprogEnabled && mw::greaterOrEqual(channel_dispatcher::getUMonDac(*this), channel_dispatcher::getUProtectionLevel(*this), getPrecision(UNIT_VOLT)));
        delay = prot_conf.u_delay;
        delay -= PROT_DELAY_CORRECTION;
    }
    else if (IS_OCP_VALUE(this, cpv)) {
        state = prot_conf.flags.i_state;
        //condition = flags.cc_mode && (!flags.cv_mode || fabs(u.mon_last - u.set) >= CHANNEL_VALUE_PRECISION);
        condition = mw::greaterOrEqual(
            channel_dispatcher::getIMon(*this),
            channel_dispatcher::getISet(*this),
            getPrecision(UNIT_AMPER));
        delay = prot_conf.i_delay;
        delay -= PROT_DELAY_CORRECTION;
    }
    else {
        state = prot_conf.flags.p_state;
        condition = channel_dispatcher::getUMon(*this) * channel_dispatcher::getIMon(*this) > channel_dispatcher::getPowerProtectionLevel(*this);
        delay = prot_conf.p_delay;
    }

    if (state && isOutputEnabled() && condition) {
        if (delay > 0) {
            if (cpv.flags.alarmed) {
                if (micros() - cpv.alarm_started >= delay * 1000000UL) {
                    cpv.flags.alarmed = 0;

                    //if (IS_OVP_VALUE(this, cpv)) {
                    //    DebugTraceF("OVP condition: CV_MODE=%d, CC_MODE=%d, I DIFF=%d mA, I MON=%d mA", (int)flags.cvMode, (int)flags.ccMode, (int)(fabs(i.mon_last - i.set) * 1000), (int)(i.mon_last * 1000));
                    //}
                    //else if (IS_OCP_VALUE(this, cpv)) {
                    //    DebugTraceF("OCP condition: CC_MODE=%d, CV_MODE=%d, U DIFF=%d mV", (int)flags.ccMode, (int)flags.cvMode, (int)(fabs(u.mon_last - u.set) * 1000));
                    //}

                    protectionEnter(cpv);
                }
            }
            else {
                cpv.flags.alarmed = 1;
                cpv.alarm_started = micros();
            }
        }
        else {
            //if (IS_OVP_VALUE(this, cpv)) {
            //    DebugTraceF("OVP condition: CV_MODE=%d, CC_MODE=%d, I DIFF=%d mA", (int)flags.cvMode, (int)flags.ccMode, (int)(fabs(i.mon_last - i.set) * 1000));
            //}
            //else if (IS_OCP_VALUE(this, cpv)) {
            //    DebugTraceF("OCP condition: CC_MODE=%d, CV_MODE=%d, U DIFF=%d mV", (int)flags.ccMode, (int)flags.cvMode, (int)(fabs(u.mon_last - u.set) * 1000));
            //}

            protectionEnter(cpv);
        }
    }
    else {
        cpv.flags.alarmed = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////

void Channel::init() {
    bool last_save_enabled = profile::enableSave(false);

    ioexp.init();
    adc.init();
    dac.init();

    profile::enableSave(last_save_enabled);
}

void Channel::onPowerDown() {
    bool last_save_enabled = profile::enableSave(false);

    outputEnable(false);
    doRemoteSensingEnable(false);
    if (getFeatures() & CH_FEATURE_RPROG) {
        doRemoteProgrammingEnable(false);
    }
    if (getFeatures() & CH_FEATURE_LRIPPLE) {
        doLowRippleEnable(false);
    }

    clearProtection(false);

    profile::enableSave(last_save_enabled);
}

void Channel::reset() {
    flags.outputEnabled = 0;
    flags.dpOn = 0;
    flags.senseEnabled = 0;
    flags.rprogEnabled = 0;

    flags.cvMode = 0;
    flags.ccMode = 0;

    ovp.flags.tripped = 0;
    ovp.flags.alarmed = 0;

    ocp.flags.tripped = 0;
    ocp.flags.alarmed = 0;

    opp.flags.tripped = 0;
    opp.flags.alarmed = 0;

    flags.currentCurrentRange = CURRENT_RANGE_HIGH;
    flags.currentRangeSelectionMode = CURRENT_RANGE_SELECTION_USE_BOTH;
    flags.autoSelectCurrentRange = 1;

    // CAL:STAT ON if valid calibrating data for both voltage and current exists in the nonvolatile memory, otherwise OFF.
    doCalibrationEnable(isCalibrationExists());

    // OUTP:PROT:CLE OFF
    // [SOUR[n]]:VOLT:PROT:TRIP? 0
    // [SOUR[n]]:CURR:PROT:TRIP? 0
    // [SOUR[n]]:POW:PROT:TRIP? 0
    clearProtection(false);

    // [SOUR[n]]:VOLT:SENS INTernal
    doRemoteSensingEnable(false);

    if (getFeatures() & CH_FEATURE_RPROG) {
        // [SOUR[n]]:VOLT:PROG INTernal
        doRemoteProgrammingEnable(false);
    }

    if (getFeatures() & CH_FEATURE_LRIPPLE) {
        doLowRippleEnable(false);
    }

    // [SOUR[n]]:VOLT:PROT:DEL
    // [SOUR[n]]:VOLT:PROT:STAT
    // [SOUR[n]]:CURR:PROT:DEL
    // [SOUR[n]]:CURR:PROT:STAT
    // [SOUR[n]]:POW:PROT[:LEV]
    // [SOUR[n]]:POW:PROT:DEL
    // [SOUR[n]]:POW:PROT:STAT -> set all to default
    clearProtectionConf();

    // [SOUR[n]]:CURR
    // [SOUR[n]]:CURR:STEP
    // [SOUR[n]]:VOLT
    // [SOUR[n]]:VOLT:STEP -> set all to default
    u.init(U_MIN, U_DEF_STEP, u.max);
    i.init(I_MIN, I_DEF_STEP, i.max);

    maxCurrentLimitCause = MAX_CURRENT_LIMIT_CAUSE_NONE;
    p_limit = PTOT;

    resetHistory();

    flags.displayValue1 = DISPLAY_VALUE_VOLTAGE;
    flags.displayValue2 = DISPLAY_VALUE_CURRENT;
    ytViewRate = GUI_YT_VIEW_RATE_DEFAULT;

    flags.voltageTriggerMode = TRIGGER_MODE_FIXED;
    flags.currentTriggerMode = TRIGGER_MODE_FIXED;
    flags.triggerOutputState = 1;
    flags.triggerOnListStop = TRIGGER_ON_LIST_STOP_OUTPUT_OFF;
    trigger::setVoltage(*this, U_MIN);
    trigger::setCurrent(*this, I_MIN);
    list::resetChannelList(*this);

#ifdef EEZ_PLATFORM_SIMULATOR
    simulator.setLoadEnabled(false);
    simulator.load = 10;
#endif
}

void Channel::resetHistory() {
    historyPosition = -1;
}

void Channel::clearCalibrationConf() {
    cal_conf.flags.u_cal_params_exists = 0;
    cal_conf.flags.i_cal_params_exists_range_high = 0;
    cal_conf.flags.i_cal_params_exists_range_low = 0;

    cal_conf.u.min.dac = cal_conf.u.min.val = cal_conf.u.min.adc = U_CAL_VAL_MIN;
    cal_conf.u.mid.dac = cal_conf.u.mid.val = cal_conf.u.mid.adc = (U_CAL_VAL_MIN + U_CAL_VAL_MAX) / 2;
    cal_conf.u.max.dac = cal_conf.u.max.val = cal_conf.u.max.adc = U_CAL_VAL_MAX;
    cal_conf.u.minPossible = U_MIN;
    cal_conf.u.maxPossible = U_MAX;

    cal_conf.i[0].min.dac = cal_conf.i[0].min.val = cal_conf.i[0].min.adc = I_CAL_VAL_MIN;
    cal_conf.i[0].mid.dac = cal_conf.i[0].mid.val = cal_conf.i[0].mid.adc = (I_CAL_VAL_MIN + I_CAL_VAL_MAX) / 2;
    cal_conf.i[0].max.dac = cal_conf.i[0].max.val = cal_conf.i[0].max.adc = I_CAL_VAL_MAX;
    cal_conf.i[0].minPossible = I_MIN;
    cal_conf.i[0].maxPossible = I_MAX;

    cal_conf.i[1].min.dac = cal_conf.i[1].min.val = cal_conf.i[1].min.adc = I_CAL_VAL_MIN / 10;
    cal_conf.i[1].mid.dac = cal_conf.i[1].mid.val = cal_conf.i[1].mid.adc = (I_CAL_VAL_MIN + I_CAL_VAL_MAX) / 2 / 10;
    cal_conf.i[1].max.dac = cal_conf.i[1].max.val = cal_conf.i[1].max.adc = I_CAL_VAL_MAX / 10;
    cal_conf.i[1].minPossible = I_MIN;
    cal_conf.i[1].maxPossible = I_MAX / 10;

    strcpy(cal_conf.calibration_date, "");
    strcpy(cal_conf.calibration_remark, CALIBRATION_REMARK_INIT);
}

void Channel::clearProtectionConf() {
    prot_conf.flags.u_state = OVP_DEFAULT_STATE;
    prot_conf.flags.i_state = OCP_DEFAULT_STATE;
    prot_conf.flags.p_state = OPP_DEFAULT_STATE;

    prot_conf.u_delay = OVP_DEFAULT_DELAY;
    prot_conf.u_level = u.max;
    prot_conf.i_delay = OCP_DEFAULT_DELAY;
    prot_conf.p_delay = OPP_DEFAULT_DELAY;
    prot_conf.p_level = OPP_DEFAULT_LEVEL;

    temperature::sensors[temp_sensor::CH1 + index - 1].prot_conf.state = OTP_CH_DEFAULT_STATE;
    temperature::sensors[temp_sensor::CH1 + index - 1].prot_conf.level = OTP_CH_DEFAULT_LEVEL;
    temperature::sensors[temp_sensor::CH1 + index - 1].prot_conf.delay = OTP_CH_DEFAULT_DELAY;
}

bool Channel::test() {
    bool last_save_enabled = profile::enableSave(false);

    flags.powerOk = 0;

    outputEnable(false);
    doRemoteSensingEnable(false);
    if (getFeatures() & CH_FEATURE_RPROG) {
        doRemoteProgrammingEnable(false);
    }
    if (getFeatures() & CH_FEATURE_LRIPPLE) {
        doLowRippleEnable(false);
    }

    ioexp.test();
    adc.test();
    dac.test();

    profile::enableSave(last_save_enabled);
    profile::save();

    return isOk();
}

bool Channel::isPowerOk() {
    return flags.powerOk;
}

bool Channel::isTestFailed() {
    return ioexp.g_testResult == psu::TEST_FAILED ||
        adc.g_testResult == psu::TEST_FAILED ||
        dac.g_testResult == psu::TEST_FAILED;
}

bool Channel::isTestOk() {
    return ioexp.g_testResult == psu::TEST_OK &&
        adc.g_testResult == psu::TEST_OK &&
        dac.g_testResult == psu::TEST_OK;
}

bool Channel::isOk() {
    return psu::isPowerUp() && isPowerOk() && isTestOk();
}

void Channel::voltageBalancing() {
    //DebugTraceF("Channel voltage balancing: CH1_Umon=%f, CH2_Umon=%f", Channel::get(0).u.mon_last, Channel::get(1).u.mon_last);
    if (isNaN(uBeforeBalancing)) {
        uBeforeBalancing = u.set;
    }
    doSetVoltage((Channel::get(0).u.mon_last + Channel::get(1).u.mon_last) / 2);
}

void Channel::currentBalancing() {
    //DebugTraceF("CH%d channel current balancing: CH1_Imon=%f, CH2_Imon=%f", index, Channel::get(0).i.mon_last, Channel::get(1).i.mon_last);
    if (isNaN(iBeforeBalancing)) {
        iBeforeBalancing = i.set;
    }
    doSetCurrent((Channel::get(0).i.mon_last + Channel::get(1).i.mon_last) / 2);
}

void Channel::restoreVoltageToValueBeforeBalancing() {
    if (!isNaN(uBeforeBalancing)) {
        //DebugTraceF("Restore voltage to value before balancing: %f", uBeforeBalancing);
        profile::enableSave(false);
        setVoltage(uBeforeBalancing);
        profile::enableSave(true);
        uBeforeBalancing = NAN;
    }
}

void Channel::restoreCurrentToValueBeforeBalancing() {
    if (!isNaN(iBeforeBalancing)) {
        //DebugTraceF("Restore current to value before balancing: %f", index, iBeforeBalancing);
        profile::enableSave(false);
        setCurrent(iBeforeBalancing);
        profile::enableSave(true);
        iBeforeBalancing = NAN;
    }
}

void Channel::tick(uint32_t tick_usec) {
    if (!isOk()) {
        return;
    }

    ioexp.tick(tick_usec);
    adc.tick(tick_usec);
    onTimeCounter.tick(tick_usec);

    if (getFeatures() & CH_FEATURE_LRIPPLE) {
        lowRippleCheck(tick_usec);
    }

    // turn off DP after delay
    if (delayed_dp_off && (micros() - delayed_dp_off_start) >= DP_OFF_DELAY_PERIOD * 1000000L) {
        delayed_dp_off = false;
        doDpEnable(false);
    }

    /// Output power is monitored and if its go below DP_NEG_LEV
    /// that is negative value in Watts (default -1 W),
    /// and that condition lasts more then DP_NEG_DELAY seconds (default 5 s),
    /// down-programmer circuit has to be switched off.
    if (isOutputEnabled()) {
        if (u.mon_last * i.mon_last >= DP_NEG_LEV || tick_usec < dpNegMonitoringTime) {
            dpNegMonitoringTime = tick_usec;
        } else {
            if (tick_usec - dpNegMonitoringTime > DP_NEG_DELAY * 1000000UL) {
                if (flags.dpOn) {
                    DebugTraceF("CH%d, neg. P, DP off: %f", index, u.mon_last * i.mon_last);
                    dpNegMonitoringTime = tick_usec;
                    psu::generateError(SCPI_ERROR_CH1_DOWN_PROGRAMMER_SWITCHED_OFF + (index - 1));
                    doDpEnable(false);
                } else {
                    DebugTraceF("CH%d, neg. P, output off: %f", index, u.mon_last * i.mon_last);
                    psu::generateError(SCPI_ERROR_CH1_OUTPUT_FAULT_DETECTED - (index - 1));
                    channel_dispatcher::outputEnable(*this, false);
                }
            } else if (tick_usec - dpNegMonitoringTime > 500 * 1000UL) {
                if (flags.dpOn) {
                    if (channel_dispatcher::isSeries()) {
                        Channel& channel = Channel::get(index == 1 ? 1 : 0);
                        channel.voltageBalancing();
                        dpNegMonitoringTime = tick_usec;
                    } else if (channel_dispatcher::isParallel()) {
                        Channel& channel = Channel::get(index == 1 ? 1 : 0);
                        channel.currentBalancing();
                        dpNegMonitoringTime = tick_usec;
                    }
                }
            }
        }
    }

    // If channel output is off then test PWRGOOD here, otherwise it is tested in Channel::eventGpio method.
#if !CONF_SKIP_PWRGOOD_TEST
    if (!isOutputEnabled()) {
        testPwrgood(ioexp.readGpio());
    }
#endif

    if (historyPosition == -1) {
        uHistory[0] = roundPrec(u.mon_last, getPrecisionFromNumSignificantDecimalDigits(VOLTAGE_NUM_SIGNIFICANT_DECIMAL_DIGITS));
        iHistory[0] = roundPrec(i.mon_last, getPrecisionFromNumSignificantDecimalDigits(CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS));
        for (int i = 1; i < CHANNEL_HISTORY_SIZE; ++i) {
            uHistory[i] = 0;
            iHistory[i] = 0;
        }

        historyPosition = 1;
        historyLastTick = tick_usec;
    } else {
        uint32_t ytViewRateMicroseconds = (int)round(ytViewRate * 1000000L);

        while (tick_usec - historyLastTick >= ytViewRateMicroseconds) {
            uHistory[historyPosition] = roundPrec(u.mon_last, getPrecisionFromNumSignificantDecimalDigits(VOLTAGE_NUM_SIGNIFICANT_DECIMAL_DIGITS));
            iHistory[historyPosition] = roundPrec(i.mon_last, getPrecisionFromNumSignificantDecimalDigits(CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS));

            if (++historyPosition == CHANNEL_HISTORY_SIZE) {
                historyPosition = 0;
            }

            historyLastTick += ytViewRateMicroseconds;
        }
    }

    doAutoSelectCurrentRange(tick_usec);
}

float Channel::remapAdcDataToVoltage(int16_t adc_data) {
    return remap((float)adc_data, (float)AnalogDigitalConverter::ADC_MIN, U_MIN, (float)AnalogDigitalConverter::ADC_MAX, U_MAX_CONF);
}

float Channel::remapAdcDataToCurrent(int16_t adc_data) {
    return remap((float)adc_data, (float)AnalogDigitalConverter::ADC_MIN, I_MIN, (float)AnalogDigitalConverter::ADC_MAX, getDualRangeMax());
}

int16_t Channel::remapVoltageToAdcData(float value) {
    float adc_value = remap(value, U_MIN, (float)AnalogDigitalConverter::ADC_MIN, U_MAX, (float)AnalogDigitalConverter::ADC_MAX);
    return (int16_t)clamp(adc_value, (float)(-AnalogDigitalConverter::ADC_MAX - 1), (float)AnalogDigitalConverter::ADC_MAX);
}

int16_t Channel::remapCurrentToAdcData(float value) {
    float adc_value = remap(value, I_MIN, (float)AnalogDigitalConverter::ADC_MIN, getDualRangeMax(), (float)AnalogDigitalConverter::ADC_MAX);
    return (int16_t)clamp(adc_value, (float)(-AnalogDigitalConverter::ADC_MAX - 1), (float)AnalogDigitalConverter::ADC_MAX);
}

void Channel::adcDataIsReady(int16_t data, bool startAgain) {
    uint8_t nextStartReg0 = 0;

    switch (adc.start_reg0) {

    case AnalogDigitalConverter::ADC_REG0_READ_U_MON:
    {
#if CONF_DEBUG
        debug::g_uMon[index - 1].set(data);
#endif

        //if (greaterOrEqual(u.mon_adc, 10.0f, getPrecision(VALUE_TYPE_FLOAT_VOLT))) {
        //    if (abs(u.mon_adc - data) > negligibleAdcDiffForVoltage2) {
        //        u.mon_adc = data;
        //    }
        //} else {
        //    if (abs(u.mon_adc - data) > negligibleAdcDiffForVoltage3) {
        //        u.mon_adc = data;
        //    }
        //}
        u.mon_adc = data;

        float value;
        if (isVoltageCalibrationEnabled()) {

            value = remapAdcDataToVoltage(data);

#if !defined(EEZ_PLATFORM_SIMULATOR)
            value -= VOLTAGE_GND_OFFSET;
#endif

            value = remap(value, cal_conf.u.min.adc, cal_conf.u.min.val, cal_conf.u.max.adc, cal_conf.u.max.val);
        } else {
            value = remapAdcDataToVoltage(data);

#if !defined(EEZ_PLATFORM_SIMULATOR)
            value -= VOLTAGE_GND_OFFSET;
#endif
        }

        u.addMonValue(value);

        nextStartReg0 = AnalogDigitalConverter::ADC_REG0_READ_I_MON;
    }
    break;

    case AnalogDigitalConverter::ADC_REG0_READ_I_MON:
    {
#if CONF_DEBUG
        debug::g_iMon[index - 1].set(data);
#endif

        //if (abs(i.mon_adc - data) > negligibleAdcDiffForCurrent) {
        //    i.mon_adc = data;
        //}
        i.mon_adc = data;

        float value = remapAdcDataToCurrent(data) - getDualRangeGndOffset();

        if (isCurrentCalibrationEnabled()) {
            value = remap(value,
                cal_conf.i[flags.currentCurrentRange].min.adc,
                cal_conf.i[flags.currentCurrentRange].min.val,
                cal_conf.i[flags.currentCurrentRange].max.adc,
                cal_conf.i[flags.currentCurrentRange].max.val);
        }

        i.addMonValue(value);

        if (isOutputEnabled()) {
            if (isRemoteProgrammingEnabled()) {
                nextStartReg0 = AnalogDigitalConverter::ADC_REG0_READ_U_SET;
            }
            else {
                nextStartReg0 = AnalogDigitalConverter::ADC_REG0_READ_U_MON;
            }
        }
        else {
            u.resetMonValues();
            i.resetMonValues();

            nextStartReg0 = AnalogDigitalConverter::ADC_REG0_READ_U_SET;
        }
    }
    break;

    case AnalogDigitalConverter::ADC_REG0_READ_U_SET:
    {
#if CONF_DEBUG
        debug::g_uMonDac[index - 1].set(data);
#endif

        float value = remapAdcDataToVoltage(data);

#if !defined(EEZ_PLATFORM_SIMULATOR)
        if (!flags.rprogEnabled) {
            value -= VOLTAGE_GND_OFFSET;
        }
#endif

        //if (isVoltageCalibrationEnabled()) {
        //    u.mon_dac = remap(value, cal_conf.u.min.adc, cal_conf.u.min.val, cal_conf.u.max.adc, cal_conf.u.max.val);
        //} else {
        //    u.mon_dac = value;
        //}

        u.addMonDacValue(value);

        if (isOutputEnabled() && isRemoteProgrammingEnabled()) {
            nextStartReg0 = AnalogDigitalConverter::ADC_REG0_READ_U_MON;
        }
        else {
            nextStartReg0 = AnalogDigitalConverter::ADC_REG0_READ_I_SET;
        }
    }
    break;

    case AnalogDigitalConverter::ADC_REG0_READ_I_SET:
    {
#if CONF_DEBUG
        debug::g_iMonDac[index - 1].set(data);
#endif

        float value = remapAdcDataToCurrent(data) - getDualRangeGndOffset();

        //if (isCurrentCalibrationEnabled()) {
        //    i.mon_dac = remap(value,
        //        cal_conf.i[flags.currentCurrentRange].min.adc,
        //        cal_conf.i[flags.currentCurrentRange].min.val,
        //        cal_conf.i[flags.currentCurrentRange].max.adc,
        //        cal_conf.i[flags.currentCurrentRange].max.val);
        //} else {
        //    i.mon_dac = value;
        //}

        i.addMonDacValue(value);

        if (isOutputEnabled()) {
            nextStartReg0 = AnalogDigitalConverter::ADC_REG0_READ_U_MON;
        }
    }
    break;
    }

    if (startAgain) {
        adc.start(nextStartReg0);
    }
}

void Channel::updateCcAndCvSwitch() {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    board::cvLedSwitch(this, isCvMode());
    board::ccLedSwitch(this, isCcMode());
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
    bp::cvLedSwitch(this, isCvMode());
    bp::ccLedSwitch(this, isCcMode());
#endif
}

void Channel::setCcMode(bool cc_mode) {
    cc_mode = cc_mode && isOutputEnabled();

    if (cc_mode != flags.ccMode) {
        flags.ccMode = cc_mode;

        updateCcAndCvSwitch();

        setOperBits(OPER_ISUM_CC, cc_mode);
        setQuesBits(QUES_ISUM_VOLT, cc_mode);

        Channel::get(index == 1 ? 1 : 0).restoreCurrentToValueBeforeBalancing();
    }
}

void Channel::setCvMode(bool cv_mode) {
    cv_mode = cv_mode && isOutputEnabled();

    if (cv_mode != flags.cvMode) {
        flags.cvMode = cv_mode;

        updateCcAndCvSwitch();

        setOperBits(OPER_ISUM_CV, cv_mode);
        setQuesBits(QUES_ISUM_CURR, cv_mode);

        Channel::get(index == 1 ? 1 : 0).restoreVoltageToValueBeforeBalancing();
    }
}

void Channel::protectionCheck() {
    if (channel_dispatcher::isCoupled() && index == 2) {
        // protections of coupled channels are checked on channel 1
        return;
    }

    protectionCheck(ovp);
    protectionCheck(ocp);
    protectionCheck(opp);
}

void Channel::eventAdcData(int16_t adc_data, bool startAgain) {
    if (!psu::isPowerUp()) return;

    adcDataIsReady(adc_data, startAgain);
    protectionCheck();
}

void Channel::eventGpio(uint8_t gpio) {
    if (!isOk()) return;

#if !CONF_SKIP_PWRGOOD_TEST
    testPwrgood(gpio);
#endif

    if (getFeatures() & CH_FEATURE_RPOL) {
        unsigned rpol = !(gpio & (1 << IOExpander::IO_BIT_IN_RPOL));

        if (rpol != flags.rpol) {
            flags.rpol = rpol;
            setQuesBits(QUES_ISUM_RPOL, flags.rpol ? true : false);
        }

        if (rpol && isOutputEnabled()) {
            channel_dispatcher::outputEnable(*this, false);
            event_queue::pushEvent(event_queue::EVENT_ERROR_CH1_REMOTE_SENSE_REVERSE_POLARITY_DETECTED + index - 1);
			onProtectionTripped();
			return;
        }
    }

    if (!io_pins::isInhibited()) {
        setCvMode(gpio & (1 << IOExpander::IO_BIT_IN_CV_ACTIVE) ? true : false);
        setCcMode(gpio & (1 << IOExpander::IO_BIT_IN_CC_ACTIVE) ? true : false);
    }
}

void Channel::adcReadMonDac() {
#if ADC_USE_INTERRUPTS
    adc.start(AnalogDigitalConverter::ADC_REG0_READ_U_SET);
    delay(ADC_TIMEOUT_MS * 2);
#else
    adc.start(AnalogDigitalConverter::ADC_REG0_READ_U_SET);
    delay(ADC_TIMEOUT_MS);
    adc.tick(micros());
    delay(ADC_TIMEOUT_MS);
    adc.tick(micros());
#endif
}

void Channel::adcReadAll() {
    if (isOutputEnabled()) {
#if ADC_USE_INTERRUPTS
        adc.start(AnalogDigitalConverter::ADC_REG0_READ_U_SET);
        delay(ADC_TIMEOUT_MS * 3);
#else
        adc.start(AnalogDigitalConverter::ADC_REG0_READ_U_SET);
        delay(ADC_TIMEOUT_MS);
        adc.tick(micros());
        delay(ADC_TIMEOUT_MS);
        adc.tick(micros());
        delay(ADC_TIMEOUT_MS);
        adc.tick(micros());
        delay(ADC_TIMEOUT_MS);
        adc.tick(micros());
#endif
    } else {
#if ADC_USE_INTERRUPTS
        adc.start(AnalogDigitalConverter::ADC_REG0_READ_U_MON);
        delay(ADC_TIMEOUT_MS * 4);
#else
        adc.start(AnalogDigitalConverter::ADC_REG0_READ_U_MON);
        delay(ADC_TIMEOUT_MS);
        adc.tick(micros());
        delay(ADC_TIMEOUT_MS);
        adc.tick(micros());
        delay(ADC_TIMEOUT_MS);
        adc.tick(micros());
        delay(ADC_TIMEOUT_MS);
        adc.tick(micros());
#endif
    }
}

void Channel::doDpEnable(bool enable) {
    // DP bit is active low
    ioexp.changeBit(IOExpander::IO_BIT_OUT_DP_ENABLE, !enable);
    setOperBits(OPER_ISUM_DP_OFF, !enable);
    flags.dpOn = enable;
    if (enable) {
        dpNegMonitoringTime = micros();
    }
}

void Channel::executeOutputEnable(bool enable) {
    ioexp.changeBit(IOExpander::IO_BIT_OUT_OUTPUT_ENABLE, enable);
    setOperBits(OPER_ISUM_OE_OFF, !enable);
    bp::switchOutput(this, enable);

    if (hasSupportForCurrentDualRange()) {
        doSetCurrentRange();
    }

    if (enable) {
        if (getFeatures() & CH_FEATURE_LRIPPLE) {
            outputEnableStartTime = micros();
            delayLowRippleCheck = true;
        }

        // enable DP
        delayed_dp_off = false;
        doDpEnable(true);

        dpNegMonitoringTime = micros();
    } else {
        if (getFeatures() & CH_FEATURE_LRIPPLE) {
            doLowRippleEnable(false);
        }

        setCvMode(false);
        setCcMode(false);

        if (calibration::isEnabled()) {
            calibration::stop();
        }

        // turn off DP after some delay
        delayed_dp_off = true;
        delayed_dp_off_start = micros();
    }

    restoreVoltageToValueBeforeBalancing();
    restoreCurrentToValueBeforeBalancing();

    if (enable) {
        // start ADC conversion
        adc.start(AnalogDigitalConverter::ADC_REG0_READ_U_MON);

        onTimeCounter.start();
    } else {
        onTimeCounter.stop();
    }
}

void Channel::doOutputEnable(bool enable) {
    if (!psu::g_isBooted) {
        flags.afterBootOutputEnabled = enable;
        return;
    }

    if (enable && !isOk()) {
        return;
    }

    flags.outputEnabled = enable;

    if (!io_pins::isInhibited()) {
        executeOutputEnable(enable);
    }
}

void Channel::onInhibitedChanged(bool inhibited) {
    if (isOutputEnabled()) {
        if (inhibited) {
            executeOutputEnable(false);
        } else {
            executeOutputEnable(true);
        }
    }
}

void Channel::doRemoteSensingEnable(bool enable) {
    if (enable && !isOk()) {
        return;
    }
    flags.senseEnabled = enable;
    bp::switchSense(this, enable);
    setOperBits(OPER_ISUM_RSENS_ON, enable);
}

void Channel::doRemoteProgrammingEnable(bool enable) {
    if (enable && !isOk()) {
        return;
    }
    flags.rprogEnabled = enable;
    if (enable) {
        setVoltageLimit(u.max);
        setVoltage(u.min);
        prot_conf.u_level = u.max;
        prot_conf.flags.u_state = true;
    }
    ioexp.changeBit(ioexp.IO_BIT_OUT_EXT_PROG, enable);
    bp::switchProg(this, enable);
    setOperBits(OPER_ISUM_RPROG_ON, enable);
}

bool Channel::isLowRippleAllowed(uint32_t tick_usec) {
    if (!isOutputEnabled()) {
        return false;
    }

    if (delayLowRippleCheck) {
        if (tick_usec - outputEnableStartTime < 100 * 1000L) {
            // If Auto low ripple is enabled, channel cannot enter low ripple mode
            // even if condition for that exist before pre-regulation is not activated
            // for a short period of time (100ms). Without this transition repetive
            // changing of channel's output mode with load connected WILL result with
            // post-regulator's power mosfet damage, and overheating of down-programmer mosfet!
            return false;
        }
        delayLowRippleCheck = false;
    }

    if (i.mon_last > SOA_PREG_CURR || i.mon_last > SOA_POSTREG_PTOT / (SOA_VIN - u.mon_last)) {
        return false;
    }

    if (i.mon_last * (SOA_VIN - u.mon_last) > SOA_POSTREG_PTOT) {
        return false;
    }

    return true;
}

void Channel::lowRippleCheck(uint32_t tick_usec) {
    if (isLowRippleAllowed(tick_usec)) {
        if (!flags.lrippleEnabled) {
            if (flags.lrippleAutoEnabled) {
                doLowRippleEnable(true);
            }
        }
    } else {
        if (flags.lrippleEnabled) {
            doLowRippleEnable(false);
        }
    }
}

void Channel::doLowRippleEnable(bool enable) {
    flags.lrippleEnabled = enable;
    ioexp.changeBit(ioexp.IO_BIT_OUT_SET_100_PERCENT, !enable);
}

void Channel::doLowRippleAutoEnable(bool enable) {
    if (enable && !isOk()) {
        return;
    }
    flags.lrippleAutoEnabled = enable;
}

void Channel::update() {
    if (!isOk()) {
        return;
    }

    if (index == 1) {
        doCalibrationEnable(persist_conf::devConf.flags.ch1CalEnabled && isCalibrationExists());
    } else if (index == 2) {
        doCalibrationEnable(persist_conf::devConf.flags.ch2CalEnabled && isCalibrationExists());
    }

    bool last_save_enabled = profile::enableSave(false);

    setVoltage(u.set);
    setCurrent(i.set);
    doOutputEnable(flags.outputEnabled);
    doRemoteSensingEnable(flags.senseEnabled);
    if (getFeatures() & CH_FEATURE_RPROG) {
        doRemoteProgrammingEnable(flags.rprogEnabled);
    }

    profile::enableSave(last_save_enabled);
}

void Channel::outputEnable(bool enable) {
    if (enable != flags.outputEnabled) {
        doOutputEnable(enable);
        event_queue::pushEvent((enable ? event_queue::EVENT_INFO_CH1_OUTPUT_ENABLED :
            event_queue::EVENT_INFO_CH1_OUTPUT_DISABLED) + index - 1);
        profile::save();
    }
}

void Channel::afterBootOutputEnable() {
    if (flags.afterBootOutputEnabled) {
        doOutputEnable(true);
    }
}

bool Channel::isOutputEnabled() {
    return psu::isPowerUp() && flags.outputEnabled;
}

void Channel::doCalibrationEnable(bool enable) {
    flags._calEnabled = enable;

    if (enable) {
        u.min = floorPrec(cal_conf.u.minPossible, getPrecision(UNIT_VOLT));
        if (u.min < U_MIN) u.min = U_MIN;
        if (u.limit < u.min) u.limit = u.min;
        if (u.set < u.min) setVoltage(u.min);

        u.max = ceilPrec(cal_conf.u.maxPossible, getPrecision(UNIT_VOLT));
        if (u.max > U_MAX) u.max = U_MAX;
        if (u.set > u.max) setVoltage(u.max);
        if (u.limit > u.max) u.limit = u.max;

        i.min = floorPrec(cal_conf.i[0].minPossible, getPrecision(UNIT_AMPER));
        if (i.min < I_MIN) i.min = I_MIN;
        if (i.limit < i.min) i.limit = i.min;
        if (i.set < i.min) setCurrent(i.min);

        i.max = ceilPrec(cal_conf.i[0].maxPossible, getPrecision(UNIT_AMPER));
        if (i.max > I_MAX) i.max = I_MAX;
        if (i.limit > i.max) i.limit = i.max;
        if (i.set > i.max) setCurrent(i.max);
    } else {
        u.min = U_MIN;
        u.max = U_MAX;

        i.min = I_MIN;
        i.max = I_MAX;
    }

    u.def = u.min;
    i.def = i.min;

    setVoltage(u.set);
    setCurrent(i.set);
}

void Channel::calibrationEnable(bool enabled) {
    if (enabled != isCalibrationEnabled()) {
        doCalibrationEnable(enabled);
        event_queue::pushEvent((enabled ? event_queue::EVENT_INFO_CH1_CALIBRATION_ENABLED :
            event_queue::EVENT_WARNING_CH1_CALIBRATION_DISABLED) + index - 1);
        persist_conf::saveCalibrationEnabledFlag(*this, enabled);
    }
}

void Channel::calibrationEnableNoEvent(bool enabled) {
    if (enabled != isCalibrationEnabled()) {
        doCalibrationEnable(enabled);
    }
}

bool Channel::isCalibrationEnabled() {
    return flags._calEnabled;
}

bool Channel::isVoltageCalibrationEnabled() {
    return flags._calEnabled && cal_conf.flags.u_cal_params_exists;
}

bool Channel::isCurrentCalibrationEnabled() {
    return flags._calEnabled && (
        (flags.currentCurrentRange == CURRENT_RANGE_HIGH && cal_conf.flags.i_cal_params_exists_range_high) ||
        (flags.currentCurrentRange == CURRENT_RANGE_LOW && cal_conf.flags.i_cal_params_exists_range_low)
    );
}

void Channel::calibrationFindVoltageRange(float minDac, float minVal, float minAdc, float maxDac, float maxVal, float maxAdc, float *min, float *max) {
    if (boardRevision == CH_BOARD_REVISION_R5B6B || boardRevision == CH_BOARD_REVISION_R5B10 || boardRevision == CH_BOARD_REVISION_R5B12) {
        *min = U_MIN;
        *max = U_MAX;
        return;
    }

    flags._calEnabled = true;

    bool u_cal_params_exists = cal_conf.flags.u_cal_params_exists;
    cal_conf.flags.u_cal_params_exists = true;

    CalibrationValueConfiguration calValueConf;
    calValueConf = cal_conf.u;

    cal_conf.u.min.dac = minDac;
    cal_conf.u.min.val = minVal;
    cal_conf.u.min.adc = minAdc;
    cal_conf.u.max.dac = maxDac;
    cal_conf.u.max.val = maxVal;
    cal_conf.u.max.adc = maxAdc;

    doSetVoltage(U_MIN);
    //DebugTraceF("U_MIN=%f", U_MIN);
    delay(100);
#if !ADC_USE_INTERRUPTS
    adc.start(AnalogDigitalConverter::ADC_REG0_READ_U_MON);
    delayMicroseconds(2 * ADC_READ_TIME_US);
    adc.tick(micros());
#endif
    //DebugTraceF("DAC=%d", (int)debug::g_uDac[index-1].get());
    //DebugTraceF("MON_ADC=%d", (int)u.mon_adc);
    *min = u.mon_last;

    doSetVoltage(U_MAX);
    //DebugTraceF("U_MAX=%f", U_MAX);
    delay(200); // guard time, because without load it will require more than 15ms to jump to the max
#if !ADC_USE_INTERRUPTS
    adc.start(AnalogDigitalConverter::ADC_REG0_READ_U_MON);
    delayMicroseconds(2 * ADC_READ_TIME_US);
    adc.tick(micros());
#endif
    //DebugTraceF("DAC=%d", (int)debug::g_uDac[index-1].get());
    //DebugTraceF("MON_ADC=%d", (int)u.mon_adc);
    *max = u.mon_last;

    cal_conf.flags.u_cal_params_exists = u_cal_params_exists;
    cal_conf.u = calValueConf;

    flags._calEnabled = false;
}

void Channel::calibrationFindCurrentRange(float minDac, float minVal, float minAdc, float maxDac, float maxVal, float maxAdc, float *min, float *max) {
    if (boardRevision == CH_BOARD_REVISION_R5B6B || boardRevision == CH_BOARD_REVISION_R5B10 || boardRevision == CH_BOARD_REVISION_R5B12) {
        *min = I_MIN;
        *max = I_MAX;
        return;
    }

    flags._calEnabled = true;

    bool i_cal_params_exists = cal_conf.flags.i_cal_params_exists_range_high;
    cal_conf.flags.i_cal_params_exists_range_high = true;

    CalibrationValueConfiguration calValueConf;
    calValueConf = cal_conf.i[0];

    cal_conf.i[0].min.dac = minDac;
    cal_conf.i[0].min.val = minVal;
    cal_conf.i[0].min.adc = minAdc;
    cal_conf.i[0].max.dac = maxDac;
    cal_conf.i[0].max.val = maxVal;
    cal_conf.i[0].max.adc = maxAdc;

    doSetCurrent(I_MIN);
    //DebugTraceF("I_MIN=%f", I_MIN);
    delay(100);
#if !ADC_USE_INTERRUPTS
    adc.start(AnalogDigitalConverter::ADC_REG0_READ_I_MON);
    delayMicroseconds(2 * ADC_READ_TIME_US);
    adc.tick(micros());
#endif
    //DebugTraceF("DAC=%d", (int)debug::g_iDac[index-1].get());
    //DebugTraceF("MON_ADC=%d", (int)i.mon_adc);
    *min = i.mon_last;

    doSetCurrent(I_MAX);
    delay(100);
    //DebugTraceF("I_MAX=%f", I_MAX);
#if !ADC_USE_INTERRUPTS
    adc.start(AnalogDigitalConverter::ADC_REG0_READ_I_MON);
    delayMicroseconds(2 * ADC_READ_TIME_US);
    adc.tick(micros());
#endif
    //DebugTraceF("DAC=%d", (int)debug::g_iDac[index-1].get());
    //DebugTraceF("MON_ADC=%d", (int)i.mon_adc);
    *max = i.mon_last;

    cal_conf.flags.i_cal_params_exists_range_high = i_cal_params_exists;
    cal_conf.i[0] = calValueConf;

    flags._calEnabled = false;
}

void Channel::remoteSensingEnable(bool enable) {
    if (enable != flags.senseEnabled) {
        doRemoteSensingEnable(enable);
        event_queue::pushEvent((enable ? event_queue::EVENT_INFO_CH1_REMOTE_SENSE_ENABLED :
            event_queue::EVENT_INFO_CH1_REMOTE_SENSE_DISABLED) + index - 1);
        profile::save();
    }
}

bool Channel::isRemoteSensingEnabled() {
    return flags.senseEnabled;
}

void Channel::remoteProgrammingEnable(bool enable) {
    if (enable != flags.rprogEnabled) {
        doRemoteProgrammingEnable(enable);
        event_queue::pushEvent((enable ? event_queue::EVENT_INFO_CH1_REMOTE_PROG_ENABLED :
            event_queue::EVENT_INFO_CH1_REMOTE_PROG_DISABLED) + index - 1);
        profile::save();
    }
}

bool Channel::isRemoteProgrammingEnabled() {
    return flags.rprogEnabled;
}

bool Channel::lowRippleEnable(bool enable) {
    if (enable != flags.lrippleEnabled) {
        if (enable && !isLowRippleAllowed(micros())) {
            return false;
        }
        doLowRippleEnable(enable);
    }
    return true;
}

bool Channel::isLowRippleEnabled() {
    return flags.lrippleEnabled;
}

void Channel::lowRippleAutoEnable(bool enable) {
    if (enable != flags.lrippleAutoEnabled) {
        doLowRippleAutoEnable(enable);
    }
}

bool Channel::isLowRippleAutoEnabled() {
    return flags.lrippleAutoEnabled;
}

void Channel::doSetVoltage(float value) {
    u.set = value;
    u.mon_dac = 0;

    if (prot_conf.u_level < u.set) {
        prot_conf.u_level = u.set;
    }

    if (U_MAX != U_MAX_CONF) {
        value = remap(value, 0, 0, U_MAX_CONF, U_MAX);
    }

    if (isVoltageCalibrationEnabled()) {
        value = remap(value, cal_conf.u.min.val, cal_conf.u.min.dac, cal_conf.u.max.val, cal_conf.u.max.dac);
    }

#if !defined(EEZ_PLATFORM_SIMULATOR)
    value += VOLTAGE_GND_OFFSET;
#endif

    dac.set_voltage(value);
}

void Channel::setVoltage(float value) {
    doSetVoltage(value);

    uBeforeBalancing = NAN;
    restoreCurrentToValueBeforeBalancing();

    profile::save();
}

void Channel::doSetCurrent(float value) {
    if (hasSupportForCurrentDualRange()) {
        if (dac.isTesting()) {
            setCurrentRange(CURRENT_RANGE_HIGH);
        } else if (!calibration::isEnabled()) {
            if (flags.currentRangeSelectionMode == CURRENT_RANGE_SELECTION_USE_BOTH) {
                setCurrentRange(mw::greater(value, 0.5, getPrecision(UNIT_AMPER)) ? CURRENT_RANGE_HIGH : CURRENT_RANGE_LOW);
            } else if (flags.currentRangeSelectionMode == CURRENT_RANGE_SELECTION_ALWAYS_HIGH) {
                setCurrentRange(CURRENT_RANGE_HIGH);
            } else {
                setCurrentRange(CURRENT_RANGE_LOW);
            }
        }
    }

    i.set = value;
    i.mon_dac = 0;

    if (I_MAX != I_MAX_CONF) {
        value = remap(value, 0, 0, I_MAX_CONF, I_MAX);
    }

    if (isCurrentCalibrationEnabled()) {
        value = remap(value,
            cal_conf.i[flags.currentCurrentRange].min.val,
            cal_conf.i[flags.currentCurrentRange].min.dac,
            cal_conf.i[flags.currentCurrentRange].max.val,
            cal_conf.i[flags.currentCurrentRange].max.dac);
    }

    value += getDualRangeGndOffset();

    dac.set_current(value);
}

void Channel::setCurrent(float value) {
    doSetCurrent(value);

    iBeforeBalancing = NAN;
    restoreVoltageToValueBeforeBalancing();

    profile::save();
}

bool Channel::isCalibrationExists() {
    return (flags.currentCurrentRange == CURRENT_RANGE_HIGH && cal_conf.flags.i_cal_params_exists_range_high) ||
        (flags.currentCurrentRange == CURRENT_RANGE_LOW && cal_conf.flags.i_cal_params_exists_range_low) ||
        cal_conf.flags.u_cal_params_exists;
}

bool Channel::isTripped() {
    return ovp.flags.tripped ||
        ocp.flags.tripped ||
        opp.flags.tripped ||
        temperature::isAnySensorTripped(this);
}

void Channel::clearProtection(bool clearOTP) {
    event_queue::Event lastEvent;
    event_queue::getLastErrorEvent(&lastEvent);

    ovp.flags.tripped = 0;
    ovp.flags.alarmed = 0;
    setQuesBits(QUES_ISUM_OVP, false);
    if (lastEvent.eventId == event_queue::EVENT_ERROR_CH1_OVP_TRIPPED + 3 * (index - 1)) {
        event_queue::markAsRead();
    }

    ocp.flags.tripped = 0;
    ocp.flags.alarmed = 0;
    setQuesBits(QUES_ISUM_OCP, false);
    if (lastEvent.eventId == event_queue::EVENT_ERROR_CH1_OCP_TRIPPED + 3 * (index - 1)) {
        event_queue::markAsRead();
    }

    opp.flags.tripped = 0;
    opp.flags.alarmed = 0;
    setQuesBits(QUES_ISUM_OPP, false);
    if (lastEvent.eventId == event_queue::EVENT_ERROR_CH1_OPP_TRIPPED + 3 * (index - 1)) {
        event_queue::markAsRead();
    }

    if (clearOTP) {
        temperature::clearChannelProtection(this);
    }
}

void Channel::disableProtection() {
    if (!isTripped()) {
        prot_conf.flags.u_state = 0;
        prot_conf.flags.i_state = 0;
        prot_conf.flags.p_state = 0;
        temperature::disableChannelProtection(this);
    }
}

void Channel::setQuesBits(int bit_mask, bool on) {
	if (serial::g_testResult == TEST_OK) {
        reg_set_ques_isum_bit(&serial::g_scpiContext, this, bit_mask, on);
    }
#if OPTION_ETHERNET
    if (ethernet::g_testResult == psu::TEST_OK) {
        reg_set_ques_isum_bit(&ethernet::g_scpiContext, this, bit_mask, on);
    }
#endif
}

void Channel::setOperBits(int bit_mask, bool on) {
	if (serial::g_testResult == TEST_OK) {
        reg_set_oper_isum_bit(&serial::g_scpiContext, this, bit_mask, on);
    }
#if OPTION_ETHERNET
    if (ethernet::g_testResult == psu::TEST_OK) {
        reg_set_oper_isum_bit(&ethernet::g_scpiContext, this, bit_mask, on);
    }
#endif
}

const char *Channel::getCvModeStr() {
    if (isCvMode()) return "CV";
    else if (isCcMode()) return "CC";
    else return "UR";
}

const char *Channel::getBoardRevisionName() {
    return CH_BOARD_REVISION_NAMES[boardRevision];
}

uint16_t Channel::getFeatures() {
    return CH_BOARD_REVISION_FEATURES[boardRevision];
}

float Channel::getVoltageLimit() const {
    return u.limit;
}

float Channel::getVoltageMaxLimit() const {
    return u.max;
}

void Channel::setVoltageLimit(float limit) {
    u.limit = limit;
    if (u.set > u.limit) {
        setVoltage(u.limit);
    }
    profile::save();
}

float Channel::getCurrentLimit() const {
    return i.limit;
}

void Channel::setCurrentLimit(float limit) {
    if (limit > getMaxCurrentLimit()) {
        limit = getMaxCurrentLimit();
    }
    i.limit = limit;
    if (i.set > i.limit) {
        setCurrent(i.limit);
    }
    profile::save();
}

float Channel::getMaxCurrentLimit() const {
    if (hasSupportForCurrentDualRange() && flags.currentRangeSelectionMode == CURRENT_RANGE_SELECTION_ALWAYS_LOW) {
        return 0.5f;
    }
    return isMaxCurrentLimited() ? ERR_MAX_CURRENT : i.max;
}

bool Channel::isMaxCurrentLimited() const {
    return getMaxCurrentLimitCause() != MAX_CURRENT_LIMIT_CAUSE_NONE;
}

MaxCurrentLimitCause Channel::getMaxCurrentLimitCause() const {
    if (psu::isMaxCurrentLimited()) {
        return psu::getMaxCurrentLimitCause();
    }
    return maxCurrentLimitCause;
}

void Channel::limitMaxCurrent(MaxCurrentLimitCause cause) {
    if (cause != maxCurrentLimitCause) {
        maxCurrentLimitCause = cause;

        if (isMaxCurrentLimited()) {
            if (isOutputEnabled() && i.mon_last > ERR_MAX_CURRENT) {
                setCurrent(0);
            }

            if (i.limit > ERR_MAX_CURRENT) {
                setCurrentLimit(ERR_MAX_CURRENT);
            }
        }
    }
}

void Channel::unlimitMaxCurrent() {
    limitMaxCurrent(MAX_CURRENT_LIMIT_CAUSE_NONE);
}

float Channel::getPowerLimit() const {
    return p_limit;
}

float Channel::getPowerMaxLimit() const {
    return PTOT;
}

void Channel::setPowerLimit(float limit) {
    p_limit = limit;
    if (u.set * i.set > p_limit) {
        //setVoltage(p_limit / i.set);
        setCurrent(p_limit / u.set);
    }
    profile::save();
}

#if !CONF_SKIP_PWRGOOD_TEST
void Channel::testPwrgood(uint8_t gpio) {
    if (!(gpio & (1 << IOExpander::IO_BIT_IN_PWRGOOD))) {
        DebugTraceF("Ch%d PWRGOOD bit changed to 0", index);
        flags.powerOk = 0;
        psu::generateError(SCPI_ERROR_CH1_FAULT_DETECTED - (index - 1));
        psu::powerDownBySensor();
        return;
    }
}
#endif

TriggerMode Channel::getVoltageTriggerMode() {
    return (TriggerMode)flags.voltageTriggerMode;
}

void Channel::setVoltageTriggerMode(TriggerMode mode) {
    flags.voltageTriggerMode = mode;
}

TriggerMode Channel::getCurrentTriggerMode() {
    return (TriggerMode)flags.currentTriggerMode;
}

void Channel::setCurrentTriggerMode(TriggerMode mode) {
    flags.currentTriggerMode = mode;
}

bool Channel::getTriggerOutputState() {
    return flags.triggerOutputState ? true : false;
}

void Channel::setTriggerOutputState(bool enabled) {
    flags.triggerOutputState = enabled ? 1 : 0;
}

TriggerOnListStop Channel::getTriggerOnListStop() {
    return (TriggerOnListStop)flags.triggerOnListStop;
}

void Channel::setTriggerOnListStop(TriggerOnListStop value) {
    flags.triggerOnListStop = value;
}

float Channel::getDualRangeGndOffset() {
#ifdef EEZ_PLATFORM_SIMULATOR
    return 0;
#else
    return flags.currentCurrentRange == CURRENT_RANGE_LOW ? (CURRENT_GND_OFFSET / 10) : CURRENT_GND_OFFSET;
#endif
}

void Channel::setCurrentRangeSelectionMode(CurrentRangeSelectionMode mode) {
    flags.currentRangeSelectionMode = mode;
    profile::save();

    if (flags.currentRangeSelectionMode == CURRENT_RANGE_SELECTION_ALWAYS_LOW) {
        if (mw::greater(i.set, 0.5, getPrecision(UNIT_AMPER))) {
            i.set = 0.5;
        }

        if (mw::greater(i.limit, 0.5, getPrecision(UNIT_AMPER))) {
            i.limit = 0.5;
        }
    }

    setCurrent(i.set);
}

void Channel::enableAutoSelectCurrentRange(bool enable) {
    flags.autoSelectCurrentRange = enable;
    profile::save();

    if (!flags.autoSelectCurrentRange) {
        setCurrent(i.set);
    }
}

bool Channel::isCurrentLowRangeAllowed() {
    return hasSupportForCurrentDualRange() && flags.currentRangeSelectionMode != CURRENT_RANGE_SELECTION_ALWAYS_HIGH;
}

float Channel::getDualRangeMax() {
    return flags.currentCurrentRange == CURRENT_RANGE_LOW ? (I_MAX / 10) : I_MAX;
}

//void Channel::calculateNegligibleAdcDiffForCurrent() {
//    if (flags.currentCurrentRange == CURRENT_RANGE_LOW) {
//        negligibleAdcDiffForCurrent = (int)((AnalogDigitalConverter::ADC_MAX - AnalogDigitalConverter::ADC_MIN) / (2 * 10000 * (I_MAX/10 - I_MIN))) + 1;
//    } else {
//        negligibleAdcDiffForCurrent = (int)((AnalogDigitalConverter::ADC_MAX - AnalogDigitalConverter::ADC_MIN) / (2 * 1000 * (I_MAX - I_MIN))) + 1;
//    }
//}

void Channel::doSetCurrentRange() {
    if (flags.outputEnabled) {
        if (flags.currentCurrentRange == 0) {
            // 5A
            //DebugTraceF("CH%d: Switched to 5A range", (int)index);
            ioexp.changeBit(IOExpander::IO_BIT_5A, true);
            ioexp.changeBit(IOExpander::IO_BIT_500mA, false);
            //calculateNegligibleAdcDiffForCurrent();
        }
        else {
            // 500mA
            //DebugTraceF("CH%d: Switched to 500mA range", (int)index);
            ioexp.changeBit(IOExpander::IO_BIT_500mA, true);
            ioexp.changeBit(IOExpander::IO_BIT_5A, false);
            //calculateNegligibleAdcDiffForCurrent();
        }
    } else {
        ioexp.changeBit(IOExpander::IO_BIT_5A, false);
        ioexp.changeBit(IOExpander::IO_BIT_500mA, false);
    }
}

void Channel::setCurrentRange(uint8_t currentCurrentRange) {
    if (hasSupportForCurrentDualRange()) {
        if (currentCurrentRange != flags.currentCurrentRange) {
            flags.currentCurrentRange = currentCurrentRange;
            doSetCurrentRange();
            if (isOutputEnabled()) {
                adc.start(AnalogDigitalConverter::ADC_REG0_READ_U_MON);
            }
        }
    }
}

void Channel::doAutoSelectCurrentRange(uint32_t tickCount) {
    if (isOutputEnabled()) {
        if (autoRangeCheckLastTickCount != 0) {
            if (tickCount - autoRangeCheckLastTickCount > CURRENT_AUTO_RANGE_SWITCHING_DELAY_MS * 1000L) {
                if (flags.autoSelectCurrentRange && flags.currentRangeSelectionMode == CURRENT_RANGE_SELECTION_USE_BOTH && hasSupportForCurrentDualRange() && !dac.isTesting() && !calibration::isEnabled()) {
                    if (flags.currentCurrentRange == CURRENT_RANGE_LOW) {
                        if (mw::greater(i.set, 0.5, getPrecision(UNIT_AMPER)) && isCcMode()) {
                            doSetCurrent(i.set);
                        }
                    } else if (i.mon_measured) {
                        if (mw::less(i.mon_last, 0.5, getPrecision(UNIT_AMPER))) {
                            setCurrentRange(1);
                            dac.set_current((uint16_t)65535);
                        }
                    }
                }
                autoRangeCheckLastTickCount = tickCount;
            }
        } else {
            autoRangeCheckLastTickCount = tickCount;
        }
    } else {
        autoRangeCheckLastTickCount = 0;
    }
}

}
} // namespace eez::psu
