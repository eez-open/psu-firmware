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

#include "eez/app/psu.h"
#include "eez/app/scpi/psu.h"

#include "eez/app/calibration.h"
#include "eez/app/trigger.h"
#include "eez/app/channel_dispatcher.h"

#if OPTION_DISPLAY
#include "eez/mw/gui/gui.h"
#endif

namespace eez {
namespace app {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_choice_def_t calibration_level_choice[] = {
    { "MINimum", calibration::LEVEL_MIN },
    { "MIDdle",  calibration::LEVEL_MID },
    { "MAXimum", calibration::LEVEL_MAX },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

////////////////////////////////////////////////////////////////////////////////

scpi_choice_def_t calibration_current_range_choice[] = {
    { "HIGH",    calibration::CURRENT_RANGE_5A },
    { "LOW", calibration::CURRENT_RANGE_500MA },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

////////////////////////////////////////////////////////////////////////////////

static scpi_result_t calibration_level(scpi_t * context, calibration::Value &calibrationValue) {
    if (!calibration::isEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CALIBRATION_STATE_IS_OFF);
        return SCPI_RES_ERR;
    }

    int32_t level;
    if (!SCPI_ParamChoice(context, calibration_level_choice, &level, true)) {
        return SCPI_RES_ERR;
    }

    if ((level == calibration::LEVEL_MID || level == calibration::LEVEL_MAX) && !calibrationValue.min_set)
    {
        SCPI_ErrorPush(context, SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS);
        return SCPI_RES_ERR;
    }

    calibrationValue.setLevel(level);
    calibrationValue.setLevelValue();

    return SCPI_RES_OK;
}
static scpi_result_t calibration_data(scpi_t * context, calibration::Value &calibrationValue) {
    if (!calibration::isEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CALIBRATION_STATE_IS_OFF);
        return SCPI_RES_ERR;
    }

    if (calibrationValue.level == calibration::LEVEL_NONE) {
        SCPI_ErrorPush(context, SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS);
        return SCPI_RES_ERR;
    }

    scpi_number_t param;
    if (!SCPI_ParamNumber(context, 0, &param, true)) {
        return SCPI_RES_ERR;
    }

    if (param.unit != SCPI_UNIT_NONE && param.unit != (calibrationValue.voltOrCurr ? SCPI_UNIT_VOLT : SCPI_UNIT_AMPER)) {
        SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
        return SCPI_RES_ERR;
    }

    float dac = calibrationValue.getDacValue();
	float value = (float)param.value;
    float adc = calibrationValue.getAdcValue();

	if (!calibrationValue.checkRange(dac, value, adc)) {
        SCPI_ErrorPush(context, SCPI_ERROR_CAL_VALUE_OUT_OF_RANGE);
        return SCPI_RES_ERR;
	}

	calibrationValue.setData(dac, value, adc);

    calibration::resetChannelToZero();

    return SCPI_RES_OK;
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_calibrationClear(scpi_t * context) {
    if (calibration::isEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS);
        return SCPI_RES_ERR;
    }

    if (!checkPassword(context, persist_conf::devConf.calibration_password)) {
        return SCPI_RES_ERR;
    }

    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    Channel *channel = &Channel::get(psu_context->selected_channel_index - 1);

    if (!calibration::clear(channel)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_calibrationMode(scpi_t * context) {
    if (channel_dispatcher::isCoupled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_CHANNELS_ARE_COUPLED);
        return SCPI_RES_ERR;
    }

    if (channel_dispatcher::isTracked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_IN_TRACKING_MODE);
        return SCPI_RES_ERR;
    }

    if (!trigger::isIdle()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
        return SCPI_RES_ERR;
    }

    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (enable == calibration::isEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS);
        return SCPI_RES_ERR;
    }

    if (!checkPassword(context, persist_conf::devConf.calibration_password)) {
        return SCPI_RES_ERR;
    }

    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    Channel *channel = &Channel::get(psu_context->selected_channel_index - 1);

    if (!channel->isOutputEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS);
        return SCPI_RES_ERR;
    }

    if (enable) {
        calibration::start(channel);
    }
    else {
        calibration::stop();
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_calibrationModeQ(scpi_t * context) {
    SCPI_ResultBool(context, calibration::isEnabled());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_calibrationCurrentData(scpi_t * context) {
    return calibration_data(context, calibration::getCurrent());
}

scpi_result_t scpi_cmd_calibrationCurrentLevel(scpi_t * context) {
    return calibration_level(context, calibration::getCurrent());
}

scpi_result_t scpi_cmd_calibrationCurrentRange(scpi_t * context) {
    if (!calibration::isEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CALIBRATION_STATE_IS_OFF);
        return SCPI_RES_ERR;
    }

    if (!calibration::hasSupportForCurrentDualRange()) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        return SCPI_RES_ERR;
    }

    int32_t range;
    if (!SCPI_ParamChoice(context, calibration_current_range_choice, &range, true)) {
        return SCPI_RES_ERR;
    }

    calibration::selectCurrentRange(range);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_calibrationPasswordNew(scpi_t * context) {
    if (!checkPassword(context, persist_conf::devConf.calibration_password)) {
        return SCPI_RES_ERR;
    }

    const char *new_password;
    size_t new_password_len;

    if (!SCPI_ParamCharacters(context, &new_password, &new_password_len, true)) {
        return SCPI_RES_ERR;
    }

	int16_t err;
	if (!persist_conf::isCalibrationPasswordValid(new_password, new_password_len, err)) {
        SCPI_ErrorPush(context, err);
        return SCPI_RES_ERR;
	}

    if (!persist_conf::changeCalibrationPassword(new_password, new_password_len)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_calibrationRemark(scpi_t * context) {
    if (!calibration::isEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CALIBRATION_STATE_IS_OFF);
        return SCPI_RES_ERR;
    }

    const char *remark;
    size_t len;
    if (!SCPI_ParamCharacters(context, &remark, &len, true)) {
        return SCPI_RES_ERR;
    }

    if (len > CALIBRATION_REMARK_MAX_LENGTH) {
        SCPI_ErrorPush(context, SCPI_ERROR_TOO_MUCH_DATA);
        return SCPI_RES_ERR;
    }

    calibration::setRemark(remark, len);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_calibrationRemarkQ(scpi_t * context) {
    const char *remark;

    if (calibration::isEnabled()) {
        remark = calibration::getRemark();
    }
    else {
        scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
        Channel *channel = &Channel::get(psu_context->selected_channel_index - 1);
        remark = channel->cal_conf.calibration_remark;
    }

    SCPI_ResultText(context, remark);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_calibrationSave(scpi_t * context) {
	int16_t err;
	if (!calibration::canSave(err)) {
        SCPI_ErrorPush(context, err);
        return SCPI_RES_ERR;
	}

    if (!calibration::save()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_calibrationState(scpi_t * context) {
    if (channel_dispatcher::isCoupled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_CHANNELS_ARE_COUPLED);
        return SCPI_RES_ERR;
    }

    if (channel_dispatcher::isTracked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_IN_TRACKING_MODE);
        return SCPI_RES_ERR;
    }

    if (calibration::isEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS);
        return SCPI_RES_ERR;
    }

    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    Channel *channel = &Channel::get(psu_context->selected_channel_index - 1);

    if (!channel->isCalibrationExists()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CAL_PARAMS_MISSING);
        return SCPI_RES_ERR;
    }

    bool calibrationEnabled;
    if (!SCPI_ParamBool(context, &calibrationEnabled, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (calibrationEnabled == channel->isCalibrationEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS);
        return SCPI_RES_ERR;
    }

	channel->calibrationEnable(calibrationEnabled);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_calibrationStateQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    Channel *channel = &Channel::get(psu_context->selected_channel_index - 1);

    SCPI_ResultBool(context, channel->isCalibrationEnabled());

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_calibrationVoltageData(scpi_t * context) {
    return calibration_data(context, calibration::getVoltage());
}

scpi_result_t scpi_cmd_calibrationVoltageLevel(scpi_t * context) {
    return calibration_level(context, calibration::getVoltage());;
}

scpi_result_t scpi_cmd_calibrationScreenInit(scpi_t * context) {
#if OPTION_DISPLAY
	mw::gui::showPage(app::gui::PAGE_ID_SCREEN_CALIBRATION_INTRO);
	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

}
}
} // namespace eez::app::scpi