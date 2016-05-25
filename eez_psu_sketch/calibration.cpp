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

static Channel *channel;
static bool enabled;
static bool remark_set;
static char remark[CALIBRATION_REMARK_MAX_LENGTH + 1];

Value voltage(true);
Value current(false);

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
        (channel->U_CAL_VAL_MAX - channel->U_CAL_VAL_MIN) :
        (channel->I_CAL_VAL_MAX - channel->I_CAL_VAL_MIN);
}

float Value::getLevelValue() {
    if (voltOrCurr) {
        if (level == LEVEL_MIN) {
            return channel->U_CAL_VAL_MIN;
        }
        else if (level == LEVEL_MID) {
            return channel->U_CAL_VAL_MID;
        }
        else {
            return channel->U_CAL_VAL_MAX;
        }
    }
    else {
        if (level == LEVEL_MIN) {
            return channel->I_CAL_VAL_MIN;
        }
        else if (level == LEVEL_MID) {
            return channel->I_CAL_VAL_MID;
        }
        else {
            return channel->I_CAL_VAL_MAX;
        }
    }
}

float Value::getAdcValue() {
    return voltOrCurr ? channel->u.mon : channel->i.mon;
}

void Value::setLevel(int8_t value) {
    level = value;

    if (voltOrCurr) {
        channel->setVoltage(getLevelValue());
        channel->setCurrent(channel->I_VOLT_CAL);
    }
    else {
        channel->setCurrent(getLevelValue());
        channel->setVoltage(channel->U_CURR_CAL);
    }
}

void Value::setData(float data, float adc) {
    if (level == LEVEL_MIN) {
        min_set = true;
        min = data;
        min_adc = adc;
    }
    else if (level == LEVEL_MID) {
        mid_set = true;
        mid = data;
        mid_adc = adc;
    }
    else {
        max_set = true;
        max = data;
        max_adc = adc;
    }
}

bool Value::checkMid() {
    float mid;

    if (voltOrCurr) {
        mid = util::remap(channel->U_CAL_VAL_MID,
            channel->U_CAL_VAL_MIN, min, channel->U_CAL_VAL_MAX, max);
    }
    else {
        mid = util::remap(channel->I_CAL_VAL_MID,
            channel->I_CAL_VAL_MIN, min, channel->I_CAL_VAL_MAX, max);
    }

    return fabsf(mid - mid) <= CALIBRATION_MID_TOLERANCE_PERCENT * (max - min) / 100.0f;
}

////////////////////////////////////////////////////////////////////////////////

void resetChannelToZero() {
    channel->setVoltage(0);
    channel->setCurrent(0);
}

bool isEnabled() {
    return enabled; 
}

void start(Channel *channel_) {
    if (enabled) return;

    enabled = true;
    channel = channel_;
    remark_set = false;
    remark[0] = 0;

    current.reset();
    voltage.reset();

    channel->flags.cal_enabled = 0;

    resetChannelToZero();

    channel->setOperBits(OPER_ISUM_CALI, true);
}

void stop() {
    if (!enabled) return;

    enabled = false;

    if (channel->isCalibrationExists()) {
        channel->flags.cal_enabled = 1;
    }

    resetChannelToZero();

    channel->setOperBits(OPER_ISUM_CALI, false);
}

bool isRemarkSet() { 
    return remark_set; 
}

const char *getRemark() { 
    return remark; 
}

void setRemark(const char *value, size_t len) {
    remark_set = true;
    memset(remark, 0, sizeof(remark));
    strncpy(remark, value, len);
}

bool save() {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    if (datetime::getDate(year, month, day)) {
        sprintf_P(channel->cal_conf.calibration_date, PSTR("%d%02d%02d"), (int)(2000 + year), (int)month, (int)day);
    }
    else {
        strcpy(channel->cal_conf.calibration_date, "");
    }

    memset(&channel->cal_conf.calibration_remark, 0, sizeof(channel->cal_conf.calibration_remark));
    strcpy(channel->cal_conf.calibration_remark, remark);

    if (voltage.min_set && voltage.mid_set && voltage.max_set) {
        channel->cal_conf.flags.u_cal_params_exists = 1;

        channel->cal_conf.u.min.dac = channel->U_CAL_VAL_MIN;
        channel->cal_conf.u.min.val = voltage.min;
        channel->cal_conf.u.min.adc = voltage.min_adc;

        channel->cal_conf.u.mid.dac = channel->U_CAL_VAL_MID;
        channel->cal_conf.u.mid.val = voltage.mid;
        channel->cal_conf.u.mid.adc = voltage.mid_adc;

        channel->cal_conf.u.max.dac = channel->U_CAL_VAL_MAX;
        channel->cal_conf.u.max.val = voltage.max;
        channel->cal_conf.u.max.adc = voltage.max_adc;

        voltage.level = LEVEL_NONE;
    }

    if (current.min_set && current.mid_set && current.max_set) {
        channel->cal_conf.flags.i_cal_params_exists = 1;

        channel->cal_conf.i.min.dac = channel->I_CAL_VAL_MIN;
        channel->cal_conf.i.min.val = current.min;
        channel->cal_conf.i.min.adc = current.min_adc;

        channel->cal_conf.i.mid.dac = channel->I_CAL_VAL_MID;
        channel->cal_conf.i.mid.val = current.mid;
        channel->cal_conf.i.mid.adc = current.mid_adc;

        channel->cal_conf.i.max.dac = channel->I_CAL_VAL_MAX;
        channel->cal_conf.i.max.val = current.max;
        channel->cal_conf.i.max.adc = current.max_adc;

        current.level = LEVEL_NONE;
    }

    resetChannelToZero();

    return persist_conf::saveChannelCalibration(channel);
}

bool clear() {
    channel->clearCalibrationConf();
    channel->flags.cal_enabled = 0;
    return persist_conf::saveChannelCalibration(channel);
}

}
}
} // namespace eez::psu::calibration