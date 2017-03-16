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
#include "calibration.h"
#include "scpi_psu.h"
#include "datetime.h"
#include "channel_dispatcher.h"

namespace eez {
namespace psu {
namespace calibration {

////////////////////////////////////////////////////////////////////////////////

static Channel *g_channel;
static bool g_enabled;
static bool g_remarkSet;
static char g_remark[CALIBRATION_REMARK_MAX_LENGTH + 1];

static int8_t g_currentRangeSelected = 0;

static Value g_voltage(true);
static Value g_currents[] = {
    Value(false, 0),
    Value(false, 1)
};

////////////////////////////////////////////////////////////////////////////////

Value::Value(bool voltOrCurr_, int currentRange_)
    : voltOrCurr(voltOrCurr_)
    , currentRange(currentRange_)
{
}

void Value::reset() {
    level = LEVEL_NONE;

    min_set = false;
    mid_set = false;
    max_set = false;
}

float Value::getRange() {
    float range;

    if (voltOrCurr) {
        range = g_channel->U_CAL_VAL_MAX - g_channel->U_CAL_VAL_MIN;
    } else {
        range = g_channel->I_CAL_VAL_MAX - g_channel->I_CAL_VAL_MIN;
        if (currentRange == 1) {
            range /= 10;
        }
    }

    return range;
}

bool Value::checkRange(float value, float adc) {
    float levelValue = getLevelValue();
    float range = getRange();

    float allowedDiff = range * CALIBRATION_DATA_TOLERANCE / 100;
    float diff;

    diff = fabsf(levelValue - value);
    if (diff > allowedDiff) {
        DebugTraceF("Data check failed: level=%f, data=%f, diff=%f, allowedDiff=%f", levelValue, value, diff, allowedDiff);
        return false;
    }

    diff = fabsf(levelValue - adc);
    if (diff > allowedDiff) {
        DebugTraceF("ADC check failed: level=%f, adc=%f, diff=%f, allowedDiff=%f", levelValue, adc, diff, allowedDiff);
        return false;
    }

    return true;
}

float Value::getLevelValue() {
    if (voltOrCurr) {
        if (level == LEVEL_MIN) {
            return g_channel->U_CAL_VAL_MIN;
        } else if (level == LEVEL_MID) {
            return g_channel->U_CAL_VAL_MID;
        } else {
            return g_channel->U_CAL_VAL_MAX;
        }
    }
    else {
        float value;
        
        if (level == LEVEL_MIN) {
            value = g_channel->I_CAL_VAL_MIN;
        } else if (level == LEVEL_MID) {
            value = g_channel->I_CAL_VAL_MID;
        } else {
            value = g_channel->I_CAL_VAL_MAX;
        }

        if (currentRange == 1) {
            value /= 10;
        }

        return value;
    }
}

float Value::getAdcValue() {
    return voltOrCurr ? g_channel->u.mon : g_channel->i.mon;
}

void Value::setLevel(int8_t value) {
    level = value;

    if (voltOrCurr) {
        g_channel->setVoltage(getLevelValue());
        g_channel->setCurrent(g_channel->I_VOLT_CAL);
    }
    else {
        g_channel->setCurrent(getLevelValue());
        g_channel->setVoltage(g_channel->U_CURR_CAL);
    }
}

void Value::setData(float data, float adc) {
    if (level == LEVEL_MIN) {
        min_set = true;
        min_val = data;
        min_adc = adc;
    }
    else if (level == LEVEL_MID) {
        mid_set = true;
        mid_val = data;
        mid_adc = adc;
    }
    else {
        max_set = true;
        max_val = data;
        max_adc = adc;
    }

    if (min_set && max_set) {
        if (voltOrCurr) {
            g_channel->calibrationFindVoltageRange(g_channel->U_CAL_VAL_MIN, min_val, min_adc, g_channel->U_CAL_VAL_MAX, max_val, max_adc, &minPossible, &maxPossible);
            DebugTraceF("Voltage range: %lf - %lfV", minPossible, maxPossible);
        }
        else {
            if (currentRange == 0) {
                g_channel->calibrationFindCurrentRange(g_channel->I_CAL_VAL_MIN, min_val, min_adc, g_channel->I_CAL_VAL_MAX, max_val, max_adc, &minPossible, &maxPossible);
                DebugTraceF("Current range: %lf - %lfA", minPossible, maxPossible);
            }
        }
    }
}

bool Value::checkMid() {
    float mid;

    if (voltOrCurr) {
        mid = util::remap(g_channel->U_CAL_VAL_MID,
            g_channel->U_CAL_VAL_MIN, min_val, g_channel->U_CAL_VAL_MAX, max_val);
    }
    else {
        if (currentRange == 0) {
            mid = util::remap(g_channel->I_CAL_VAL_MID,
                g_channel->I_CAL_VAL_MIN, min_val, g_channel->I_CAL_VAL_MAX, max_val);
        } else {
            mid = util::remap(g_channel->I_CAL_VAL_MID / 10,
                g_channel->I_CAL_VAL_MIN / 10, min_val, g_channel->I_CAL_VAL_MAX / 10, max_val);
        }
    }

    float allowedDiff = CALIBRATION_MID_TOLERANCE_PERCENT * (max_val - min_val) / 100.0f;

    float diff = fabsf(mid - mid_val);
    if (fabsf(mid - mid_val) <= allowedDiff) {
        return true;
    } else {
        DebugTraceF("MID point check failed: mid_level=%f, mid_data=%f, diff=%f, allowedDiff=%f",
            mid, mid_val, diff, allowedDiff);
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////

void resetChannelToZero() {
    g_channel->setVoltage(g_channel->u.min);
    g_channel->setCurrent(g_channel->i.min);
}

bool isEnabled() {
    return g_enabled; 
}

void start(Channel *channel_) {
    if (g_enabled) return;

    g_enabled = true;
    g_channel = channel_;
    g_currentRangeSelected = 0;
    g_channel->setCurrentRange(g_currentRangeSelected);
    g_remarkSet = false;
    g_remark[0] = 0;

    g_currents[0].reset();
    if (currentHasDualRange()) {
        g_currents[1].reset();
    }
    g_voltage.reset();

    g_channel->calibrationEnable(false);
    resetChannelToZero();

    g_channel->setOperBits(OPER_ISUM_CALI, true);
}

void stop() {
    if (!g_enabled) return;

    g_enabled = false;

    if (g_channel->isCalibrationExists()) {
        g_channel->calibrationEnable(true);
    }
    resetChannelToZero();

    g_channel->setOperBits(OPER_ISUM_CALI, false);
}

bool currentHasDualRange() {
    return g_channel->boardRevision == CH_BOARD_REVISION_R5B12;
}

void selectCurrentRange(int8_t range) {
    g_currentRangeSelected = range;
    g_channel->setCurrentRange(range);
}

Value& getVoltage() {
    return g_voltage;
}

Value& getCurrent() {
    return g_currents[g_currentRangeSelected];
}

bool isRemarkSet() { 
    return g_remarkSet; 
}

const char *getRemark() { 
    return g_remark; 
}

void setRemark(const char *value, size_t len) {
    g_remarkSet = true;
    memset(g_remark, 0, sizeof(g_remark));
    strncpy(g_remark, value, len);
}

static bool checkCalibrationValue(calibration::Value &calibrationValue, int16_t &scpiErr) {
    if (calibrationValue.min_val >= calibrationValue.max_val) {
        scpiErr = SCPI_ERROR_INVALID_CAL_DATA;
        return false;
    }

    if (!calibrationValue.checkMid()) {
        scpiErr = SCPI_ERROR_INVALID_CAL_DATA;
        return false;
    }

    return true;
}

bool isVoltageCalibrated() {
    return g_voltage.min_set && g_voltage.mid_set && g_voltage.max_set;
}

bool isCurrentCalibrated(Value &current) {
    return current.min_set && current.mid_set && current.max_set;
}

bool canSave(int16_t &scpiErr) {
    if (!isEnabled()) {
        scpiErr = SCPI_ERROR_CALIBRATION_STATE_IS_OFF;
        return false;
    }

    if (!isRemarkSet()) {
        scpiErr = SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS;
        return false;
    }

    // at least one value should be calibrated
    bool valueCalibrated = false;
    
    if (isVoltageCalibrated()) {
        if (!checkCalibrationValue(calibration::g_voltage, scpiErr)) {
            return false;
        }
        valueCalibrated = true;
    }

    if (isCurrentCalibrated(g_currents[0])) {
        if (!checkCalibrationValue(g_currents[0], scpiErr)) {
            return false;
        }
        valueCalibrated = true;
    }

    if (currentHasDualRange()) {
        if (isCurrentCalibrated(g_currents[1])) {
            if (!checkCalibrationValue(g_currents[1], scpiErr)) {
                return false;
            }
            valueCalibrated = true;
        }
    }

    if (!valueCalibrated) {
        scpiErr = SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS;
        return false;
    }

    return true;
}

bool save() {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    if (datetime::getDate(year, month, day)) {
        sprintf_P(g_channel->cal_conf.calibration_date, PSTR("%d%02d%02d"), (int)(2000 + year), (int)month, (int)day);
    }
    else {
        strcpy(g_channel->cal_conf.calibration_date, "");
    }

    memset(&g_channel->cal_conf.calibration_remark, 0, sizeof(g_channel->cal_conf.calibration_remark));
    strcpy(g_channel->cal_conf.calibration_remark, g_remark);

    if (isVoltageCalibrated()) {
        g_channel->cal_conf.flags.u_cal_params_exists = 1;

        g_channel->cal_conf.u.min.dac = g_channel->U_CAL_VAL_MIN;
        g_channel->cal_conf.u.min.val = g_voltage.min_val;
        g_channel->cal_conf.u.min.adc = g_voltage.min_adc;

        g_channel->cal_conf.u.mid.dac = g_channel->U_CAL_VAL_MID;
        g_channel->cal_conf.u.mid.val = g_voltage.mid_val;
        g_channel->cal_conf.u.mid.adc = g_voltage.mid_adc;

        g_channel->cal_conf.u.max.dac = g_channel->U_CAL_VAL_MAX;
        g_channel->cal_conf.u.max.val = g_voltage.max_val;
        g_channel->cal_conf.u.max.adc = g_voltage.max_adc;

        g_channel->cal_conf.u.minPossible = g_voltage.minPossible;
        g_channel->cal_conf.u.maxPossible = g_voltage.maxPossible;

        g_voltage.level = LEVEL_NONE;
    }

    if (isCurrentCalibrated(g_currents[0])) {
        g_channel->cal_conf.flags.i_cal_params_exists_range0 = 1;

        g_channel->cal_conf.i[0].min.dac = g_channel->I_CAL_VAL_MIN;
        g_channel->cal_conf.i[0].min.val = g_currents[0].min_val;
        g_channel->cal_conf.i[0].min.adc = g_currents[0].min_adc;

        g_channel->cal_conf.i[0].mid.dac = g_channel->I_CAL_VAL_MID;
        g_channel->cal_conf.i[0].mid.val = g_currents[0].mid_val;
        g_channel->cal_conf.i[0].mid.adc = g_currents[0].mid_adc;

        g_channel->cal_conf.i[0].max.dac = g_channel->I_CAL_VAL_MAX;
        g_channel->cal_conf.i[0].max.val = g_currents[0].max_val;
        g_channel->cal_conf.i[0].max.adc = g_currents[0].max_adc;

        g_channel->cal_conf.i[0].minPossible = g_currents[0].minPossible;
        g_channel->cal_conf.i[0].maxPossible = g_currents[0].maxPossible;

        g_currents[0].level = LEVEL_NONE;
    }

    if (currentHasDualRange()) {
        if (isCurrentCalibrated(g_currents[1])) {
            g_channel->cal_conf.flags.i_cal_params_exists_range1 = 1;

            g_channel->cal_conf.i[1].min.dac = g_channel->I_CAL_VAL_MIN / 10;
            g_channel->cal_conf.i[1].min.val = g_currents[1].min_val;
            g_channel->cal_conf.i[1].min.adc = g_currents[1].min_adc;

            g_channel->cal_conf.i[1].mid.dac = g_channel->I_CAL_VAL_MID / 10;
            g_channel->cal_conf.i[1].mid.val = g_currents[1].mid_val;
            g_channel->cal_conf.i[1].mid.adc = g_currents[1].mid_adc;

            g_channel->cal_conf.i[1].max.dac = g_channel->I_CAL_VAL_MAX / 10;
            g_channel->cal_conf.i[1].max.val = g_currents[1].max_val;
            g_channel->cal_conf.i[1].max.adc = g_currents[1].max_adc;

            g_channel->cal_conf.i[1].minPossible = g_currents[1].minPossible;
            g_channel->cal_conf.i[1].maxPossible = g_currents[1].maxPossible;

            g_currents[1].level = LEVEL_NONE;
        }
    }

    resetChannelToZero();

    return persist_conf::saveChannelCalibration(g_channel);
}

bool clear(Channel *channel) {
    channel->calibrationEnable(false);
    channel->clearCalibrationConf();
    return persist_conf::saveChannelCalibration(channel);
}

}
}
} // namespace eez::psu::calibration