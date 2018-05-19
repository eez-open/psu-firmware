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
#include "eez/app/temp_sensor.h"
#include "eez/app/channel_dispatcher.h"

namespace eez {
namespace app {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

static scpi_choice_def_t channel_choice[] = {
    { "CH1", 1 },
    { "CH2", 2 },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

#define TEMP_SENSOR(NAME, INSTALLED, PIN, CAL_POINTS, CH_NUM, QUES_REG_BIT, SCPI_ERROR) { #NAME, temp_sensor::NAME }

scpi_choice_def_t temp_sensor_choice[] = {
	TEMP_SENSORS,
    SCPI_CHOICE_LIST_END /* termination of option list */
};

#undef TEMP_SENSOR

scpi_choice_def_t internal_external_choice[] = {
    { "INTernal", 0 },
    { "EXTernal", 1 },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

////////////////////////////////////////////////////////////////////////////////

bool check_channel(scpi_t *context, int32_t ch) {
    if (ch < 1 || ch > CH_NUM) {
        SCPI_ErrorPush(context, SCPI_ERROR_CHANNEL_NOT_FOUND);
        return false;
    }

    if (!isPowerUp()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return false;
    }

    if (Channel::get(ch - 1).isTestFailed()) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_ERROR);
        return false;
    }

    if (!Channel::get(ch - 1).isPowerOk()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CH1_FAULT_DETECTED - (ch - 1));
        return false;
    }

    if (!Channel::get(ch - 1).isOk()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return false;
    }

    return true;
}

Channel *param_channel(scpi_t *context, scpi_bool_t mandatory, scpi_bool_t skip_channel_check) {
    int32_t ch;

    if (!SCPI_ParamChoice(context, channel_choice, &ch, mandatory)) {
        if (mandatory || SCPI_ParamErrorOccurred(context)) {
            return 0;
        }
        scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
        ch = psu_context->selected_channel_index;
    }

    if (!skip_channel_check && !check_channel(context, ch)) return 0;

    return &Channel::get(ch - 1);
}

Channel *set_channel_from_command_number(scpi_t *context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    int32_t ch;
    SCPI_CommandNumbers(context, &ch, 1, psu_context->selected_channel_index);
    if (ch < 1 || ch > CH_NUM) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return 0;
    }

    if (!check_channel(context, ch)) return 0;

    return &Channel::get(ch - 1);
}

bool param_temp_sensor(scpi_t *context, int32_t &sensor) {
    if (!SCPI_ParamChoice(context, temp_sensor_choice, &sensor, FALSE)) {
#if OPTION_AUX_TEMP_SENSOR
        if (SCPI_ParamErrorOccurred(context)) {
            return false;
        }
        sensor = temp_sensor::AUX;
#else
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
		return false;
#endif
    }

	if (!temp_sensor::sensors[sensor].installed) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
		return false;
	}

	return true;
}

bool get_voltage_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_voltage_from_param(context, param, value, channel, cv);
}

bool get_voltage_protection_level_param(scpi_t *context, float &value, float min, float max, float def) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_voltage_protection_level_from_param(context, param, value, min, max, def);
}

bool get_current_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_current_from_param(context, param, value, channel, cv);
}

bool get_power_param(scpi_t *context, float &value, float min, float max, float def) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_power_from_param(context, param, value, min, max, def);
}

bool get_temperature_param(scpi_t *context, float &value, float min, float max, float def) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_temperature_from_param(context, param, value, min, max, def);
}

bool get_duration_param(scpi_t *context, float &value, float min, float max, float def) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_duration_from_param(context, param, value, min, max, def);
}

bool get_voltage_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
    if (param.special) {
		if (channel) {
			if (param.tag == SCPI_NUM_MAX) {
				value = channel_dispatcher::getUMax(*channel);
			}
			else if (param.tag == SCPI_NUM_MIN) {
				value = channel_dispatcher::getUMin(*channel);
			}
			else if (param.tag == SCPI_NUM_DEF) {
				value = channel_dispatcher::getUDef(*channel);
			}
			else if (param.tag == SCPI_NUM_UP && cv) {
				value = cv->set + cv->step;
				if (value > channel_dispatcher::getUMax(*channel)) value = channel_dispatcher::getUMax(*channel);
			}
			else if (param.tag == SCPI_NUM_DOWN && cv) {
				value = cv->set - cv->step;
				if (value < channel_dispatcher::getUMin(*channel)) value = channel_dispatcher::getUMin(*channel);
			}
			else {
				SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
				return false;
			}
		} else {
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return false;
		}
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_VOLT) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;

		if (channel) {
			if (value < channel_dispatcher::getUMin(*channel) || value > channel_dispatcher::getUMax(*channel)) {
				SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
				return false;
			}
		}
    }

    return true;
}

bool get_voltage_protection_level_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = max;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = min;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_VOLT) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < min || value > max) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }
    return true;
}

bool get_current_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = channel_dispatcher::getIMax(*channel);
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = channel_dispatcher::getIMin(*channel);
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = channel_dispatcher::getIDef(*channel);
        }
        else if (param.tag == SCPI_NUM_UP && cv) {
            value = cv->set + cv->step;
            if (value > channel_dispatcher::getIMax(*channel)) value = channel_dispatcher::getIMax(*channel);
        }
        else if (param.tag == SCPI_NUM_DOWN && cv) {
            value = cv->set - cv->step;
            if (value < channel_dispatcher::getIMin(*channel)) value = channel_dispatcher::getIMin(*channel);
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_AMPER) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < channel_dispatcher::getIMin(*channel) || value > channel_dispatcher::getIMax(*channel)) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

bool get_power_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = max;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = min;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < min || value > max) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }
    return true;
}

bool get_temperature_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = max;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = min;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_CELSIUS) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < min || value > max) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

bool get_duration_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = max;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = min;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_SECOND) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < min || value > max) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

bool get_voltage_limit_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_voltage_limit_from_param(context, param, value, channel, cv);
}

bool get_current_limit_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_current_limit_from_param(context, param, value, channel, cv);
}

bool get_power_limit_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_power_limit_from_param(context, param, value, channel, cv);
}

bool get_voltage_limit_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
	if (context==NULL || channel==NULL) {
		SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
		return false;
	}
    if (param.special) {
		if (param.tag == SCPI_NUM_MAX) {
			value = channel_dispatcher::getUMaxLimit(*channel);
		}
		else if (param.tag == SCPI_NUM_MIN) {
			value = channel_dispatcher::getUMin(*channel);
		}
		else if (param.tag == SCPI_NUM_DEF) {
			value = channel_dispatcher::getUMaxLimit(*channel);
		}
		else {
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return false;
		}
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_VOLT) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;

		if (value < channel_dispatcher::getUMin(*channel) || value > channel_dispatcher::getUMaxLimit(*channel)) {
			SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
			return false;
		}
    }

    return true;
}

bool get_current_limit_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
	if (context==NULL || channel==NULL) {
		SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
		return false;
	}
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = channel_dispatcher::getIMaxLimit(*channel);
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = channel_dispatcher::getIMin(*channel);
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = channel_dispatcher::getIMaxLimit(*channel);
		} else {
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return false;
		}
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_AMPER) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;

        if (value < channel_dispatcher::getIMin(*channel) || value > channel_dispatcher::getIMaxLimit(*channel)) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

bool get_power_limit_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv) {
	if (context==NULL || channel==NULL) {
		SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
		return false;
	}
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = channel_dispatcher::getPowerMaxLimit(*channel);
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = channel_dispatcher::getPowerMinLimit(*channel);
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = channel_dispatcher::getPowerMaxLimit(*channel);
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_WATT) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;

        if (value < channel_dispatcher::getPowerMinLimit(*channel) || value > channel_dispatcher::getPowerMaxLimit(*channel)) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }

    return true;
}

scpi_result_t result_float(scpi_t *context, Channel *channel, float value, Unit unit) {
    char buffer[32] = { 0 };

    int numSignificantDecimalDigits = getNumSignificantDecimalDigits(unit);
    if (channel && channel->isCurrentLowRangeAllowed() && unit == UNIT_AMPER && mw::lessOrEqual(value, 0.5, getPrecision(UNIT_AMPER))) {
        ++numSignificantDecimalDigits;
    }

    strcatFloat(buffer, value, numSignificantDecimalDigits);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));
    return SCPI_RES_OK;
}

bool get_profile_location_param(scpi_t * context, int &location, bool all_locations) {
    int32_t param;
    if (!SCPI_ParamInt(context, &param, true)) {
        return false;
    }

    if (param < (all_locations ? 0 : 1) || param > NUM_PROFILE_LOCATIONS - 1) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return false;
    }

    location = (int)param;

    return true;
}

void outputOnTime(scpi_t* context, uint32_t time) {
    char str[128];
	ontime::counterToString(str, sizeof(str), time);
	SCPI_ResultText(context, str);
}

bool checkPassword(scpi_t *context, const char *againstPassword) {
    const char *password;
    size_t len;

    if (!SCPI_ParamCharacters(context, &password, &len, true)) {
        return false;
    }

	size_t nPassword = strlen(againstPassword);
    if (nPassword != len || strncmp(password, againstPassword, len) != 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_INVALID_PASSWORD);
        return false;
    }

    return true;
}

#if OPTION_SD_CARD

void cleanupPath(char *filePath) {
	replaceCharacter(filePath, '\\', '/');

	char *q = filePath;

	for (char *p = filePath; *p; ++p) {
		if (*p == '\\') {
			*p = '/';
		}

		if (*p == '/' && (p > filePath && *(p - 1) == '/')) {
			// '//' -> '/'
			continue;
		}
		else if (*p == '.') {
			if (!*(p + 1)) {
				// '<...>/.' -> '<...>'
				break;
			}
			else if (*(p + 1) == '/') {
				// '<...>/./<...>' -> '<...>/<...>'
				++p;
				continue;
			}
			else if (*(p + 1) == '.') {
				// '<...>/something/..<...>' -> '<...>/<...>'
				q -= 2;
				while (true) {
					if (q < filePath) {
						q = filePath;
						break;
					}
					if (*q == '/') {
						break;
					}
					--q;
				}
				++p;
				continue;
			}
		}

		*q++ = *p;
	}

	// remove trailing '/'
	if (q > filePath && *(q - 1) == '/') {
		--q;
	}

	// if empty then make it '/'
	if (q == filePath) {
		*q++ = '/';
	}

	*q = 0;
}

bool getFilePath(scpi_t *context, char *filePath, bool mandatory) {
	scpi_psu_t *psuContext = (scpi_psu_t *)context->user_context;

	const char *filePathParam;
	size_t filePathParamLen;
	if (SCPI_ParamCharacters(context, &filePathParam, &filePathParamLen, mandatory)) {
		if (filePathParamLen > MAX_PATH_LENGTH) {
			SCPI_ErrorPush(context, SCPI_ERROR_FILE_NAME_ERROR);
			return false;
		}

		// is it absolute file path?
		if (filePathParam[0] == '/' || filePathParam[0] == '\\') {
			// yes
			strncpy(filePath, filePathParam, filePathParamLen);
			filePath[filePathParamLen] = 0;
		}
		else {
			// no, combine with current directory to get absolute path
			size_t currentDirectoryLen = strlen(psuContext->currentDirectory);
			size_t filePathLen = currentDirectoryLen + 1 + filePathParamLen;
			if (filePathLen > MAX_PATH_LENGTH) {
				SCPI_ErrorPush(context, SCPI_ERROR_FILE_NAME_ERROR);
				return false;
			}
			strncpy(filePath, psuContext->currentDirectory, currentDirectoryLen);
			filePath[currentDirectoryLen] = '/';
			strncpy(filePath + currentDirectoryLen + 1, filePathParam, filePathParamLen);
			filePath[filePathLen] = 0;
		}
	}
	else {
		if (SCPI_ParamErrorOccurred(context)) {
			return false;
		}
		strcpy(filePath, psuContext->currentDirectory);
	}

	cleanupPath(filePath);

	return true;
}

#endif // OPTION_SD_CARD

}
}
} // namespace eez::app::scpi