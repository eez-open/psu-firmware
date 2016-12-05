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
#include "scpi_psu.h"
#include "scpi_cal.h"

#include "calibration.h"
#include "channel_coupling.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_choice_def_t calibration_level_choice[] = {
    { "MINimum", calibration::LEVEL_MIN },
    { "MIDdle",  calibration::LEVEL_MID },
    { "MAXimum", calibration::LEVEL_MAX },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

////////////////////////////////////////////////////////////////////////////////

static bool check_password(scpi_t * context) {
    const char *password;
    size_t len;

    if (!SCPI_ParamCharacters(context, &password, &len, true)) {
        return false;
    }

	int nPassword = strlen(persist_conf::dev_conf.calibration_password);
    if (nPassword != len || strncmp(persist_conf::dev_conf.calibration_password, password, len) != 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_INVALID_CAL_PASSWORD);
        return false;
    }

    return true;
}

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

	float value = (float)param.value;
    float adc = calibrationValue.getAdcValue();

	if (!calibrationValue.checkRange(value, adc)) {
        SCPI_ErrorPush(context, SCPI_ERROR_CAL_VALUE_OUT_OF_RANGE);
        return SCPI_RES_ERR;
	}
    
	calibrationValue.setData(value, adc);

    return SCPI_RES_OK;
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cal_Clear(scpi_t * context) {
    if (calibration::isEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS);
        return SCPI_RES_ERR;
    }

    if (!check_password(context)) {
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

scpi_result_t scpi_cal_Mode(scpi_t * context) {
    if (channel_coupling::getType() != channel_coupling::TYPE_NONE) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
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

    if (!check_password(context)) {
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

scpi_result_t scpi_cal_ModeQ(scpi_t * context) {
    SCPI_ResultBool(context, calibration::isEnabled());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cal_CurrentData(scpi_t * context) {
    return calibration_data(context, calibration::g_current);
}

scpi_result_t scpi_cal_CurrentLevel(scpi_t * context) {
    return calibration_level(context, calibration::g_current);
}

scpi_result_t scpi_cal_PasswordNew(scpi_t * context) {
    if (!check_password(context)) {
        return SCPI_RES_ERR;
    }

    const char *new_password;
    size_t new_password_len;

    if (!SCPI_ParamCharacters(context, &new_password, &new_password_len, true)) {
        return SCPI_RES_ERR;
    }

	int16_t err;
	if (!persist_conf::isPasswordValid(new_password, new_password_len, err)) {
        SCPI_ErrorPush(context, err);
        return SCPI_RES_ERR;
	}

    if (!persist_conf::changePassword(new_password, new_password_len)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cal_Remark(scpi_t * context) {
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

scpi_result_t scpi_cal_RemarkQ(scpi_t * context) {
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

scpi_result_t scpi_cal_Save(scpi_t * context) {
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

scpi_result_t scpi_cal_State(scpi_t * context) {
    if (channel_coupling::getType() != channel_coupling::TYPE_NONE) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
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

scpi_result_t scpi_cal_StateQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    Channel *channel = &Channel::get(psu_context->selected_channel_index - 1);

    SCPI_ResultBool(context, channel->isCalibrationEnabled());

    return SCPI_RES_OK;
}

scpi_result_t scpi_cal_VoltageData(scpi_t * context) {
    return calibration_data(context, calibration::g_voltage);
}

scpi_result_t scpi_cal_VoltageLevel(scpi_t * context) {
    return calibration_level(context, calibration::g_voltage);;
}

}
}
} // namespace eez::psu::scpi