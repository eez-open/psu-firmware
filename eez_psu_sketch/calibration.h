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

namespace eez {
namespace psu {
/// Channel calibration procedure.
namespace calibration {

enum Level {
    LEVEL_NONE,
    LEVEL_MIN,
    LEVEL_MID,
    LEVEL_MAX
};

enum CurrentRange {
    CURRENT_RANGE_5A,
    CURRENT_RANGE_500MA
};

/// Calibration parameters for the voltage or current during calibration procedure.
struct Value {
    bool voltOrCurr;
    int8_t currentRange;

    int8_t level;

    bool  min_set;
    float min_dac;
    float min_val;
    float min_adc;

    bool  mid_set;
    float mid_dac;
    float mid_val;
    float mid_adc;

    bool  max_set;
    float max_dac;
    float max_val;
    float max_adc;

    float minPossible;
    float maxPossible;

    Value(bool voltOrCurr, int currentRange_ = -1);

    void reset();

    float getLevelValue();
    float getAdcValue();

    void  setLevel(int8_t level);
    void setData(float data, float adc);

    bool checkRange(float value, float adc);
    bool checkMid();

private:
    float getRange();
};

bool isEnabled();
Channel &getCalibrationChannel();

/// Start calibration procedure on the channel.
/// /param channel Selected channel
void start(Channel *channel);

/// Stop calibration procedure.
void stop();

/// Set U and I to zero for the calibration channel.
void resetChannelToZero();

bool currentHasDualRange();
void selectCurrentRange(int8_t range);

Value& getVoltage();
Value& getCurrent();

/// Is calibration remark is set.
bool isRemarkSet();

/// Get currently set remark.
const char *getRemark();

/// Set calibration remark.
void setRemark(const char *value, size_t len);

/// Are all calibration parameters entered?
bool canSave(int16_t &scpiErr);

/// Save calibration parameters entered during calibration procedure.
bool save();

/// Clear calibration parameters for the currently selected channel.
/// /param channel Selected channel
bool clear(Channel *channel);

}
}
} // namespace eez::psu::calibration
