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

namespace eez {
namespace psu {
namespace calibration {

////////////////////////////////////////////////////////////////////////////////

static Channel *g_channel;
static bool g_enabled;
static bool g_remarkSet;
static char g_remark[CALIBRATION_REMARK_MAX_LENGTH + 1];

Value g_voltage(true);
Value g_current(false);

////////////////////////////////////////////////////////////////////////////////

Value::Value(bool voltOrCurr_)
    : voltOrCurr(voltOrCurr_)
{
}

void Value::reset() {
    level = LEVEL_NONE;

    min_set = false;
    mid_set = false;
    max_set = false;
}

float Value::getRange() {
    return voltOrCurr ?
        (g_channel->U_CAL_VAL_MAX - g_channel->U_CAL_VAL_MIN) :
        (g_channel->I_CAL_VAL_MAX - g_channel->I_CAL_VAL_MIN);
}

bool Value::checkRange(float value, float adc) {
	float levelValue = getLevelValue();
    float range = getRange();

    float diff;

    diff = (float)(levelValue - value);
    if (fabsf(diff) >= range * CALIBRATION_DATA_TOLERANCE / 100) {
        return false;
    }

    diff = levelValue - adc;
    if (fabsf(diff) >= range * CALIBRATION_DATA_TOLERANCE / 100) {
        return false;
    }

	return true;
}

float Value::getLevelValue() {
    if (voltOrCurr) {
        if (level == LEVEL_MIN) {
            return g_channel->U_CAL_VAL_MIN;
        }
        else if (level == LEVEL_MID) {
            return g_channel->U_CAL_VAL_MID;
        }
        else {
            return g_channel->U_CAL_VAL_MAX;
        }
    }
    else {
        if (level == LEVEL_MIN) {
            return g_channel->I_CAL_VAL_MIN;
        }
        else if (level == LEVEL_MID) {
            return g_channel->I_CAL_VAL_MID;
        }
        else {
            return g_channel->I_CAL_VAL_MAX;
        }
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
			g_channel->calibrationFindVoltageRange(g_channel->U_CAL_VAL_MIN, min_val, g_channel->U_CAL_VAL_MAX, max_val, &minPossible, &maxPossible);
			DebugTraceF("Voltage range: %lf - %lfV", minPossible, maxPossible);
		}
		else {
			g_channel->calibrationFindCurrentRange(g_channel->I_CAL_VAL_MIN, min_val, g_channel->I_CAL_VAL_MAX, max_val, &minPossible, &maxPossible);
			DebugTraceF("Current range: %lf - %lfA", minPossible, maxPossible);
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
        mid = util::remap(g_channel->I_CAL_VAL_MID,
            g_channel->I_CAL_VAL_MIN, min_val, g_channel->I_CAL_VAL_MAX, max_val);
    }

    return fabsf(mid - mid) <= CALIBRATION_MID_TOLERANCE_PERCENT * (max_val - min_val) / 100.0f;
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
    g_remarkSet = false;
    g_remark[0] = 0;

    g_current.reset();
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

bool canSave(int16_t &scpiErr) {
    if (!isEnabled()) {
        scpiErr = SCPI_ERROR_CALIBRATION_STATE_IS_OFF;
        return false;
    }

    if (!isRemarkSet()) {
        scpiErr = SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS;
        return false;
    }

    bool u_calibrated = false;
    if (g_voltage.min_set && g_voltage.mid_set && g_voltage.max_set) {
        if (!checkCalibrationValue(calibration::g_voltage, scpiErr)) {
            return false;
        }
        u_calibrated = true;
    }

    bool i_calibrated = false;
    if (g_current.min_set && g_current.mid_set && g_current.max_set) {
        if (!checkCalibrationValue(g_current, scpiErr)) {
            return false;
        }
        i_calibrated = true;
    }

    if (!u_calibrated && !i_calibrated) {
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

    if (g_voltage.min_set && g_voltage.mid_set && g_voltage.max_set) {
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

    if (g_current.min_set && g_current.mid_set && g_current.max_set) {
        g_channel->cal_conf.flags.i_cal_params_exists = 1;

        g_channel->cal_conf.i.min.dac = g_channel->I_CAL_VAL_MIN;
        g_channel->cal_conf.i.min.val = g_current.min_val;
        g_channel->cal_conf.i.min.adc = g_current.min_adc;

        g_channel->cal_conf.i.mid.dac = g_channel->I_CAL_VAL_MID;
        g_channel->cal_conf.i.mid.val = g_current.mid_val;
        g_channel->cal_conf.i.mid.adc = g_current.mid_adc;

        g_channel->cal_conf.i.max.dac = g_channel->I_CAL_VAL_MAX;
        g_channel->cal_conf.i.max.val = g_current.max_val;
        g_channel->cal_conf.i.max.adc = g_current.max_adc;

		g_channel->cal_conf.i.minPossible = g_current.minPossible;
		g_channel->cal_conf.i.maxPossible = g_current.maxPossible;

		g_current.level = LEVEL_NONE;
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