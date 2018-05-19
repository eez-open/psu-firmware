/*
* EEZ PSU Firmware
* Copyright (C) 2018-present, Envox d.o.o.
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

#if OPTION_SD_CARD
#include "eez/app/dlog.h"
#endif

namespace eez {
namespace app {
namespace scpi {

scpi_result_t scpi_cmd_abortDlog(scpi_t * context) {
#if OPTION_SD_CARD
	dlog::abort();
	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_initiateDlog(scpi_t * context) {
#if OPTION_SD_CARD
	char filePath[MAX_PATH_LENGTH + 1];
	if (!getFilePath(context, filePath, true)) {
		return SCPI_RES_ERR;
	}

	if (!dlog::isIdle()) {
		SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
		return SCPI_RES_ERR;
	}

	int result = dlog::initiate(filePath);
	if (result != SCPI_RES_OK) {
		SCPI_ErrorPush(context, result);
		return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_senseDlogFunctionVoltage(scpi_t * context) {
#if OPTION_SD_CARD
	bool enable;
	if (!SCPI_ParamBool(context, &enable, TRUE)) {
		return SCPI_RES_ERR;
	}

	Channel *channel = param_channel(context);
	if (!channel) {
		return SCPI_RES_ERR;
	}

	if (!dlog::isIdle()) {
		SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
		return SCPI_RES_ERR;
	}

	dlog::g_logVoltage[channel->index - 1] = enable;

	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_senseDlogFunctionVoltageQ(scpi_t * context) {
#if OPTION_SD_CARD
	Channel *channel = param_channel(context);
	if (!channel) {
		return SCPI_RES_ERR;
	}

	SCPI_ResultBool(context, dlog::g_logVoltage[channel->index - 1]);

	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_senseDlogFunctionCurrent(scpi_t * context) {
#if OPTION_SD_CARD
	bool enable;
	if (!SCPI_ParamBool(context, &enable, TRUE)) {
		return SCPI_RES_ERR;
	}

	Channel *channel = param_channel(context);
	if (!channel) {
		return SCPI_RES_ERR;
	}

	if (!dlog::isIdle()) {
		SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
		return SCPI_RES_ERR;
	}

	dlog::g_logCurrent[channel->index - 1] = enable;

	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_senseDlogFunctionCurrentQ(scpi_t * context) {
#if OPTION_SD_CARD
	Channel *channel = param_channel(context);
	if (!channel) {
		return SCPI_RES_ERR;
	}

	SCPI_ResultBool(context, dlog::g_logCurrent[channel->index - 1]);

	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_senseDlogFunctionPower(scpi_t * context) {
#if OPTION_SD_CARD
	bool enable;
	if (!SCPI_ParamBool(context, &enable, TRUE)) {
		return SCPI_RES_ERR;
	}

	Channel *channel = param_channel(context);
	if (!channel) {
		return SCPI_RES_ERR;
	}

	if (!dlog::isIdle()) {
		SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
		return SCPI_RES_ERR;
	}

	dlog::g_logPower[channel->index - 1] = enable;

	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_senseDlogFunctionPowerQ(scpi_t * context) {
#if OPTION_SD_CARD
	Channel *channel = param_channel(context);
	if (!channel) {
		return SCPI_RES_ERR;
	}

	SCPI_ResultBool(context, dlog::g_logPower[channel->index - 1]);

	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_senseDlogPeriod(scpi_t * context) {
#if OPTION_SD_CARD
	scpi_number_t param;
	if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
		return SCPI_RES_ERR;
	}

	float period;

	if (param.special) {
		if (param.tag == SCPI_NUM_MIN) {
			period = dlog::PERIOD_MIN;
		}
		else if (param.tag == SCPI_NUM_MAX) {
			period = dlog::PERIOD_MAX;
		}
		else if (param.tag == SCPI_NUM_DEF) {
			period = dlog::PERIOD_DEFAULT;
		}
		else {
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return SCPI_RES_ERR;
		}
	}
	else {
		if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_SECOND) {
			SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
			return SCPI_RES_ERR;
		}

		period = (float)param.value;
	}

	if (!dlog::isIdle()) {
		SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
		return SCPI_RES_ERR;
	}

	dlog::g_period = period;

	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_senseDlogPeriodQ(scpi_t * context) {
#if OPTION_SD_CARD
	SCPI_ResultFloat(context, dlog::g_period);
	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_senseDlogTime(scpi_t * context) {
#if OPTION_SD_CARD
	scpi_number_t param;
	if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
		return SCPI_RES_ERR;
	}

	float time;

	if (param.special) {
		if (param.tag == SCPI_NUM_MIN) {
			time = dlog::TIME_MIN;
		}
		else if (param.tag == SCPI_NUM_MAX) {
			time = dlog::TIME_MAX;
		}
		else if (param.tag == SCPI_NUM_DEF) {
			time = dlog::TIME_DEFAULT;
		}
		else {
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return SCPI_RES_ERR;
		}
	}
	else {
		if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_SECOND) {
			SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
			return SCPI_RES_ERR;
		}

		time = (float)param.value;
	}

	if (!dlog::isIdle()) {
		SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
		return SCPI_RES_ERR;
	}

	dlog::g_time = time;

	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_senseDlogTimeQ(scpi_t * context) {
#if OPTION_SD_CARD
	SCPI_ResultFloat(context, dlog::g_time);
	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

}
}
} // namespace eez::app::scpi