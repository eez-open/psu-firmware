/*
 * EEZ PSU Firmware
 * Copyright (C) 2016-present, Envox d.o.o.
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

#if OPTION_DISPLAY

#include "persist_conf.h"
#include "sound.h"
#include "calibration.h"
#include "channel_dispatcher.h"

#include "gui_password.h"
#include "gui_calibration.h"
#include "gui_keypad.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {
namespace calibration {

static const int MAX_STEP_NUM = 10;

int g_stepNum;

void showCurrentStep();

////////////////////////////////////////////////////////////////////////////////

void onStartPasswordOk() {
    g_channel->outputEnable(false);
    
    g_channel->clearProtection();
    g_channel->prot_conf.flags.u_state = 0;
    g_channel->prot_conf.flags.i_state = 0;
    g_channel->prot_conf.flags.p_state = 0;

    if (g_channel->getFeatures() & CH_FEATURE_RPROG) {
        g_channel->remoteProgrammingEnable(false);
    }
    if (g_channel->getFeatures() & CH_FEATURE_LRIPPLE) {
        channel_dispatcher::lowRippleEnable(*g_channel, false);
        channel_dispatcher::lowRippleAutoEnable(*g_channel, false);
    }

    g_channel->outputEnable(true);

    psu::calibration::start(g_channel);

    g_stepNum = 0;
    showCurrentStep();
}

void start() {
    checkPassword(PSTR("Password: "), persist_conf::devConf.calibration_password, onStartPasswordOk);
}

data::Value getLevelValue() {
    if (g_stepNum < 3) {
        return data::Value(psu::calibration::getVoltage().getLevelValue(), VALUE_TYPE_FLOAT_VOLT);
    }
    if (g_stepNum < 6) {
        return data::Value(psu::calibration::getCurrent().getLevelValue(), VALUE_TYPE_FLOAT_AMPER);
    }
    return data::Value(psu::calibration::getCurrent().getLevelValue(), VALUE_TYPE_FLOAT_AMPER, CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS_R5B12);
}

data::Value getData(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_CHANNEL_CALIBRATION_STATUS) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        return data::Value(channel.isCalibrationExists() ? 1 : 0);
    } else if (id == DATA_ID_CHANNEL_CALIBRATION_STATE) {
        int iChannel = cursor.i == -1 ? g_channel->index - 1 : cursor.i;
        return data::Value(Channel::get(iChannel).isCalibrationEnabled() ? 1 : 0);
    } else if (id == DATA_ID_CHANNEL_CALIBRATION_DATE) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        return data::Value(channel.cal_conf.calibration_date);
    } else if (id == DATA_ID_CHANNEL_CALIBRATION_REMARK) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        return data::Value(channel.cal_conf.calibration_remark);
    } else if (id == DATA_ID_CAL_CH_U_MIN) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        return data::Value(channel.cal_conf.u.min.val, VALUE_TYPE_FLOAT_VOLT);
    } else if (id == DATA_ID_CAL_CH_U_MID) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        return data::Value(channel.cal_conf.u.mid.val, VALUE_TYPE_FLOAT_VOLT);
    } else if (id == DATA_ID_CAL_CH_U_MAX) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        return data::Value(channel.cal_conf.u.max.val, VALUE_TYPE_FLOAT_VOLT);
    } else if (id == DATA_ID_CAL_CH_I0_MIN) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        return data::Value(channel.cal_conf.i[0].min.val, VALUE_TYPE_FLOAT_AMPER);
    } else if (id == DATA_ID_CAL_CH_I0_MID) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        return data::Value(channel.cal_conf.i[0].mid.val, VALUE_TYPE_FLOAT_AMPER);
    } else if (id == DATA_ID_CAL_CH_I0_MAX) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        return data::Value(channel.cal_conf.i[0].max.val, VALUE_TYPE_FLOAT_AMPER);
    } else if (id == DATA_ID_CAL_CH_I1_MIN) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        if (channel.boardRevision == CH_BOARD_REVISION_R5B12) {
            return data::Value(channel.cal_conf.i[1].min.val, VALUE_TYPE_FLOAT_AMPER, CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS_R5B12);
        } else {
            return data::Value(PSTR(""));
        }
    } else if (id == DATA_ID_CAL_CH_I1_MID) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        if (channel.boardRevision == CH_BOARD_REVISION_R5B12) {
            return data::Value(channel.cal_conf.i[1].mid.val, VALUE_TYPE_FLOAT_AMPER, CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS_R5B12);
        } else {
            return data::Value(PSTR(""));
        }
    } else if (id == DATA_ID_CAL_CH_I1_MAX) {
        Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
        if (channel.boardRevision == CH_BOARD_REVISION_R5B12) {
            return data::Value(channel.cal_conf.i[1].max.val, VALUE_TYPE_FLOAT_AMPER, CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS_R5B12);
        } else {
            return data::Value(PSTR(""));
        }
    } else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_NUM) {
        return data::Value(g_stepNum);
    } else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_LEVEL_VALUE) {
        return getLevelValue();
    } else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_VALUE) {
        switch (g_stepNum) {
        case 0: return data::Value(psu::calibration::getVoltage().min_val, VALUE_TYPE_FLOAT_VOLT);
        case 1: return data::Value(psu::calibration::getVoltage().mid_val, VALUE_TYPE_FLOAT_VOLT);
        case 2: return data::Value(psu::calibration::getVoltage().max_val, VALUE_TYPE_FLOAT_VOLT);
        case 3: return data::Value(psu::calibration::getCurrent().min_val, VALUE_TYPE_FLOAT_AMPER);
        case 4: return data::Value(psu::calibration::getCurrent().mid_val, VALUE_TYPE_FLOAT_AMPER);
        case 5: return data::Value(psu::calibration::getCurrent().max_val, VALUE_TYPE_FLOAT_AMPER);
        case 6: return data::Value(psu::calibration::getCurrent().min_val, VALUE_TYPE_FLOAT_AMPER, CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS_R5B12);
        case 7: return data::Value(psu::calibration::getCurrent().mid_val, VALUE_TYPE_FLOAT_AMPER, CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS_R5B12);
        case 8: return data::Value(psu::calibration::getCurrent().max_val, VALUE_TYPE_FLOAT_AMPER, CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS_R5B12);
        case 9: return data::Value(psu::calibration::getRemark());
        }
    } else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_STATUS) {
        switch (g_stepNum) {
        case 0: return data::Value(psu::calibration::getVoltage().min_set ? 1 : 0);
        case 1: return data::Value(psu::calibration::getVoltage().mid_set ? 1 : 0);
        case 2: return data::Value(psu::calibration::getVoltage().max_set ? 1 : 0);
        case 3: case 6: return data::Value(psu::calibration::getCurrent().min_set ? 1 : 0);
        case 4: case 7: return data::Value(psu::calibration::getCurrent().mid_set ? 1 : 0);
        case 5: case 8: return data::Value(psu::calibration::getCurrent().max_set ? 1 : 0);
        case 9: return data::Value(psu::calibration::isRemarkSet() ? 1 : 0);
        }
    } else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_PREV_ENABLED) {
        return data::Value(g_stepNum > 0 ? 1 : 0);
    } else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_NEXT_ENABLED) {
        return data::Value(g_stepNum < MAX_STEP_NUM ? 1 : 0);
    } else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_IS_SET_REMARK_STEP) {
        return data::Value(g_stepNum == MAX_STEP_NUM - 1 ? 1 : 0);
    }

    return data::Value();
}

void showCurrentStep() {
    psu::calibration::getCalibrationChannel().setCurrent(0);
    psu::calibration::getCalibrationChannel().setVoltage(0);

    if (g_stepNum < MAX_STEP_NUM) { 
        switch (g_stepNum) {
        case 0: psu::calibration::getVoltage().setLevel(psu::calibration::LEVEL_MIN); break;
        case 1: psu::calibration::getVoltage().setLevel(psu::calibration::LEVEL_MID); break;
        case 2: psu::calibration::getVoltage().setLevel(psu::calibration::LEVEL_MAX); break;
        case 3: psu::calibration::selectCurrentRange(0); psu::calibration::getCurrent().setLevel(psu::calibration::LEVEL_MIN); break;
        case 4: psu::calibration::selectCurrentRange(0); psu::calibration::getCurrent().setLevel(psu::calibration::LEVEL_MID); break;
        case 5: psu::calibration::selectCurrentRange(0); psu::calibration::getCurrent().setLevel(psu::calibration::LEVEL_MAX); break;
        case 6: psu::calibration::selectCurrentRange(1); psu::calibration::getCurrent().setLevel(psu::calibration::LEVEL_MIN); break;
        case 7: psu::calibration::selectCurrentRange(1); psu::calibration::getCurrent().setLevel(psu::calibration::LEVEL_MID); break;
        case 8: psu::calibration::selectCurrentRange(1); psu::calibration::getCurrent().setLevel(psu::calibration::LEVEL_MAX); break;
        }

        replacePage(PAGE_ID_SYS_SETTINGS_CAL_CH_WIZ_STEP);
    } else {
        replacePage(PAGE_ID_SYS_SETTINGS_CAL_CH_WIZ_FINISH);
     }
}

psu::calibration::Value *getCalibrationValue() {
    if (g_stepNum < 3) {
        return &psu::calibration::getVoltage();
    }
    return &psu::calibration::getCurrent();
}


void onSetLevelOk(float value) {
    getCalibrationValue()->setDacValue(value);

    popPage();
}

void setLevelValue() {
    data::Value levelValue = getLevelValue();

    NumericKeypadOptions options;

    options.editUnit = levelValue.getType();

    if (levelValue.getType() == VALUE_TYPE_FLOAT_VOLT) {
        options.min = g_channel->u.min;
        options.max = g_channel->u.max;
    } else {
        options.min = g_channel->i.min;
        options.max = g_channel->i.max;
    }

    options.def = 0;

    options.flags.signButtonEnabled = true;
    options.flags.dotButtonEnabled = true;

    NumericKeypad *numericKeypad = NumericKeypad::start(0, levelValue, options, onSetLevelOk, showCurrentStep);

    if (g_stepNum == 0 || g_stepNum == 3 || g_stepNum >= 6 && g_stepNum <= 8) {
        numericKeypad->switchToMilli();
    }
}

void onSetOk(float value) {
    psu::calibration::Value *calibrationValue = getCalibrationValue();

    float dac = calibrationValue->getDacValue();
    float adc = calibrationValue->getAdcValue();
    if (calibrationValue->checkRange(dac, value, adc)) {
        calibrationValue->setData(dac, value, adc);

        popPage();
        nextStep();
    } else {
        errorMessageP(PSTR("Value out of range!"));
    }
}

bool canSave() {
    int16_t scpiErr;
    return psu::calibration::canSave(scpiErr);
}

void onSetRemarkOk(char *remark) {
    psu::calibration::setRemark(remark, strlen(remark));
    popPage();
    if (g_stepNum < MAX_STEP_NUM - 1) {
        nextStep();
    } else {
        if (canSave()) {
            nextStep();
        } else {
            showCurrentStep();
        }
    }
}

void set() {
    if (!psu::calibration::isEnabled()) {
        setPage(PAGE_ID_MAIN);
        return;
    }

    if (g_stepNum < MAX_STEP_NUM - 1) {
        if (g_stepNum < 3) {
            psu::calibration::getVoltage().setLevelValue();
        } else {
            psu::calibration::getCurrent().setLevelValue();
        }

        psu::calibration::Value *calibrationValue = getCalibrationValue();

        NumericKeypadOptions options;

        if (calibrationValue == &psu::calibration::getVoltage()) {
            options.editUnit = VALUE_TYPE_FLOAT_VOLT;

            options.min = g_channel->u.min;
            options.max = g_channel->u.max;

        } else {
            options.editUnit = VALUE_TYPE_FLOAT_AMPER;

            options.min = g_channel->i.min;
            options.max = g_channel->i.max;
        }

        options.def = 0;

        options.flags.signButtonEnabled = true;
        options.flags.dotButtonEnabled = true;

        NumericKeypad *numericKeypad = NumericKeypad::start(0, data::Value(), options, onSetOk, showCurrentStep);

        if (g_stepNum == 0 || g_stepNum == 3 || g_stepNum >= 6 && g_stepNum <= 8) {
            numericKeypad->switchToMilli();
        }
    } else if (g_stepNum == MAX_STEP_NUM - 1) {
        psu::calibration::resetChannelToZero();
        Keypad::startPush(0, psu::calibration::isRemarkSet() ? psu::calibration::getRemark() : 0, CALIBRATION_REMARK_MAX_LENGTH, false, onSetRemarkOk, popPage);
    }
}

void previousStep() {
    if (!psu::calibration::isEnabled()) {
        setPage(PAGE_ID_MAIN);
        return;
    }

    if (g_stepNum > 0) {
        --g_stepNum;
        if (g_stepNum == 8 && !psu::calibration::currentHasDualRange()) {
            g_stepNum = 5;
        }
        showCurrentStep();
    }
}

void nextStep() {
    if (!psu::calibration::isEnabled()) {
        setPage(PAGE_ID_MAIN);
        return;
    }

    if (g_stepNum == MAX_STEP_NUM - 1) {
        if (!canSave()) {
            errorMessageP(PSTR("Missing calibration data!"));
            return;
        }
    }

    ++g_stepNum;
    if (g_stepNum == 6 && !psu::calibration::currentHasDualRange()) {
        g_stepNum = 9;
    }
    showCurrentStep();
}

void save() {
    if (psu::calibration::save()) {
        psu::calibration::stop();
        infoMessageP(PSTR("Calibration data saved!"), popPage);
    } else {
        errorMessageP(PSTR("Save failed!"));
    }
}

void stop() {
    psu::calibration::stop();

    g_channel->outputEnable(false);
    
    g_channel->prot_conf.flags.u_state = g_channel->OVP_DEFAULT_STATE;
    g_channel->prot_conf.flags.i_state = g_channel->OCP_DEFAULT_STATE;
    g_channel->prot_conf.flags.p_state = g_channel->OPP_DEFAULT_STATE;
}

void toggleEnable() {
    Channel &channel = g_channel ? *g_channel : Channel::get(g_foundWidgetAtDown.cursor.i);
    channel.calibrationEnable(!channel.isCalibrationEnabled());
}

}
}
}
} // namespace eez::psu::gui::calibration

#endif