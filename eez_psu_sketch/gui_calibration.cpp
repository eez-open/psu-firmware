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

#include "persist_conf.h"
#include "sound.h"
#include "calibration.h"

#include "gui_data_snapshot.h"
#include "gui_calibration.h"
#include "gui_keypad.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {
namespace calibration {

static void (*g_checkPasswordOkCallback)();
static void (*g_checkPasswordInvalidCallback)();

static char g_newPassword[PASSWORD_MAX_LENGTH];

int g_stepNum;

////////////////////////////////////////////////////////////////////////////////

void checkPasswordOkCallback(char *text) {
	int nPassword = strlen(persist_conf::dev_conf.calibration_password);
	int nText = strlen(text);
	if (nPassword == nText && strncmp(persist_conf::dev_conf.calibration_password, text, nText) == 0) {
		g_checkPasswordOkCallback();
	} else {
		// entered password doesn't match, 
		errorMessageP(PSTR("Invalid password!"), g_checkPasswordInvalidCallback);
	}
}

void checkPassword(const char *label, void (*ok)(), void (*invalid)()) {
	g_checkPasswordOkCallback = ok;
	g_checkPasswordInvalidCallback = invalid;
	keypad::start(label, 0, PASSWORD_MAX_LENGTH, true, checkPasswordOkCallback, invalid);
}

////////////////////////////////////////////////////////////////////////////////

void onRetypeNewPasswordOk(char *text) {
	if (strncmp(g_newPassword, text, strlen(text)) != 0) {
		// retyped new password doesn't match
		errorMessageP(PSTR("Password doesn't match!"), showPreviousPage);
		return;
	}
	
	if (!persist_conf::changePassword(g_newPassword, strlen(g_newPassword))) {
		// failed to save changed password
		errorMessageP(PSTR("Failed to change password!"), showPreviousPage);
		return;
	}

	// success
	infoMessageP(PSTR("Password changed!"), showPreviousPage);
}

void onNewPasswordOk(char *text) {
	int textLength = strlen(text);

	int16_t err;
	if (!persist_conf::isPasswordValid(text, textLength, err)) {
		// invalid password (probably too short), return to keypad
		errorMessageP(PSTR("Password too short!"));
		return;
	}

	strcpy(g_newPassword, text);
	keypad::start(PSTR("Retype new password: "), 0, PASSWORD_MAX_LENGTH, true, onRetypeNewPasswordOk, showPreviousPage);
}

void onOldPasswordOk() {
	keypad::start(PSTR("New password: "), 0, PASSWORD_MAX_LENGTH, true, onNewPasswordOk, showPreviousPage);
}

void editPassword() {
	checkPassword(PSTR("Current password: "), onOldPasswordOk, showPreviousPage);
}

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
        g_channel->lowRippleEnable(false);
		g_channel->lowRippleAutoEnable(false);
    }

	g_channel->outputEnable(true);

	psu::calibration::start(g_channel);

	g_stepNum = 0;
	showPage(PAGE_ID_SYS_SETTINGS_CAL_CH_WIZ_STEP, false);
}

void start() {
	checkPassword(PSTR("Password: "), onStartPasswordOk, showPreviousPage);
}

data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_CHANNEL_CALIBRATION_STATUS) {
		Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
		return data::Value(channel.isCalibrationExists() ? 1 : 0);
	} else if (id == DATA_ID_CHANNEL_CALIBRATION_STATE) {
		int iChannel = cursor.i == -1 ? g_channel->index - 1 : cursor.i;
		return data::Value(snapshot->channelSnapshots[iChannel].flags.cal_enabled ? 1 : 0);
	} else if (id == DATA_ID_CHANNEL_CALIBRATION_DATE) {
		Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
		return data::Value(channel.cal_conf.calibration_date);
	} else if (id == DATA_ID_CHANNEL_CALIBRATION_REMARK) {
		Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
		return data::Value(channel.cal_conf.calibration_remark);
	} else if (id == DATA_ID_CAL_CH_U_MIN) {
		Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
		return data::Value(channel.cal_conf.u.min.val, data::VALUE_TYPE_FLOAT_VOLT);
	} else if (id == DATA_ID_CAL_CH_U_MID) {
		Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
		return data::Value(channel.cal_conf.u.mid.val, data::VALUE_TYPE_FLOAT_VOLT);
	} else if (id == DATA_ID_CAL_CH_U_MAX) {
		Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
		return data::Value(channel.cal_conf.u.max.val, data::VALUE_TYPE_FLOAT_VOLT);
	} else if (id == DATA_ID_CAL_CH_I_MIN) {
		Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
		return data::Value(channel.cal_conf.i.min.val, data::VALUE_TYPE_FLOAT_AMPER);
	} else if (id == DATA_ID_CAL_CH_I_MID) {
		Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
		return data::Value(channel.cal_conf.i.mid.val, data::VALUE_TYPE_FLOAT_AMPER);
	} else if (id == DATA_ID_CAL_CH_I_MAX) {
		Channel &channel = cursor.i == -1 ? *g_channel : Channel::get(cursor.i);
		return data::Value(channel.cal_conf.i.max.val, data::VALUE_TYPE_FLOAT_AMPER);
	} else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_NUM) {
		return data::Value(g_stepNum);
	} else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_VALUE) {
		switch (g_stepNum) {
		case 0: return data::Value(psu::calibration::voltage.min, data::VALUE_TYPE_FLOAT_VOLT);
		case 1: return data::Value(psu::calibration::voltage.mid, data::VALUE_TYPE_FLOAT_VOLT);
		case 2: return data::Value(psu::calibration::voltage.max, data::VALUE_TYPE_FLOAT_VOLT);
		case 3: return data::Value(psu::calibration::current.min, data::VALUE_TYPE_FLOAT_AMPER);
		case 4: return data::Value(psu::calibration::current.mid, data::VALUE_TYPE_FLOAT_AMPER);
		case 5: return data::Value(psu::calibration::current.max, data::VALUE_TYPE_FLOAT_AMPER);
		case 6: return data::Value(psu::calibration::getRemark());
		}
	} else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_STATUS) {
		switch (g_stepNum) {
		case 0: return data::Value(psu::calibration::voltage.min_set ? 1 : 0);
		case 1: return data::Value(psu::calibration::voltage.mid_set ? 1 : 0);
		case 2: return data::Value(psu::calibration::voltage.max_set ? 1 : 0);
		case 3: return data::Value(psu::calibration::current.min_set ? 1 : 0);
		case 4: return data::Value(psu::calibration::current.mid_set ? 1 : 0);
		case 5: return data::Value(psu::calibration::current.max_set ? 1 : 0);
		case 6: return data::Value(psu::calibration::isRemarkSet() ? 1 : 0);
		}
	} else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_PREV_ENABLED) {
		return data::Value(g_stepNum > 0 ? 1 : 0);
	} else if (id == DATA_ID_CHANNEL_CALIBRATION_STEP_NEXT_ENABLED) {
		return data::Value(g_stepNum < MAX_STEP_NUM ? 1 : 0);
	}

	return data::Value();
}

psu::calibration::Value *getCalibrationValue() {
	if (g_stepNum < 3) {
		return &psu::calibration::voltage;
	}
	return &psu::calibration::current;
}

void onSetOk(float value) {
	psu::calibration::Value *calibrationValue = getCalibrationValue();

    float adc = calibrationValue->getAdcValue();
	if (calibrationValue->checkRange(value, adc)) {
		calibrationValue->setData(value, adc);

		showPreviousPage();
		nextStep();
	} else {
		errorMessageP(PSTR("Value out of range!"));
	}
}

void showCurrentStep() {
	if (g_stepNum < 7) {
		showPage(PAGE_ID_SYS_SETTINGS_CAL_CH_WIZ_STEP, false);
	} else {
		showPage(PAGE_ID_SYS_SETTINGS_CAL_CH_WIZ_FINISH, false);
	}
}

void onSetRemarkOk(char *remark) {
	psu::calibration::setRemark(remark, strlen(remark));
	if (g_stepNum < 6) {
		nextStep();
	} else {
		int16_t scpiErr;
		if (psu::calibration::canSave(scpiErr)) {
			nextStep();
		} else {
			showCurrentStep();
		}
	}
}

void set() {
	if (g_stepNum < 6) {
		switch (g_stepNum) {
		case 0: psu::calibration::voltage.setLevel(psu::calibration::LEVEL_MIN); break;
		case 1: psu::calibration::voltage.setLevel(psu::calibration::LEVEL_MID); break;
		case 2: psu::calibration::voltage.setLevel(psu::calibration::LEVEL_MAX); break;
		case 3: psu::calibration::current.setLevel(psu::calibration::LEVEL_MIN); break;
		case 4: psu::calibration::current.setLevel(psu::calibration::LEVEL_MID); break;
		case 5: psu::calibration::current.setLevel(psu::calibration::LEVEL_MAX); break;
		}
	
		psu::calibration::Value *calibrationValue = getCalibrationValue();

		numeric_keypad::Options options;

		if (calibrationValue == &psu::calibration::voltage) {
			options.editUnit = data::VALUE_TYPE_FLOAT_VOLT;

			options.min = g_channel->U_MIN;
			options.max = g_channel->U_MAX;

		} else {
			options.editUnit = data::VALUE_TYPE_FLOAT_AMPER;

			options.min = g_channel->I_MIN;
			options.max = g_channel->I_MAX;
		}

		options.def = 0;

		options.flags.genericNumberKeypad = true;
		options.flags.maxButtonEnabled = false;
		options.flags.defButtonEnabled = false;
		options.flags.signButtonEnabled = true;
		options.flags.dotButtonEnabled = true;


		numeric_keypad::start(0, options, onSetOk, showCurrentStep);

		if (g_stepNum == 0 || g_stepNum == 3) {
			numeric_keypad::switchToMilli();
		}
	} else if (g_stepNum == 6) {
		keypad::start(0, psu::calibration::isRemarkSet() ? psu::calibration::getRemark() : 0, 32, false, onSetRemarkOk, showCurrentStep);
	}
}

void previousStep() {
	if (g_stepNum > 0) {
		--g_stepNum;
		showPage(PAGE_ID_SYS_SETTINGS_CAL_CH_WIZ_STEP, false);
	}
}

void nextStep() {
	if (g_stepNum == 6) {
		int16_t scpiErr;
		if (!psu::calibration::canSave(scpiErr)) {
			if (scpiErr == SCPI_ERROR_INVALID_CAL_DATA) {
				errorMessageP(PSTR("Invalid calibration data!"));
			} else {
				errorMessageP(PSTR("Missing calibration data!"));
			}
			return;
		}
	}

	++g_stepNum;

	showCurrentStep();
}

void save() {
	if (psu::calibration::save()) {
		infoMessageP(PSTR("Calibration data saved!"), showPreviousPage);
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
	channel.calibrationEnable(!channel.flags.cal_enabled);
}

}
}
}
} // namespace eez::psu::gui::calibration
