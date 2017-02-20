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

#include "calibration.h"
#include "channel_dispatcher.h"
#include "profile.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_choice_def_t channelsCouplingChoice[] = {
    { "NONE", channel_dispatcher::TYPE_NONE },
    { "PARallel", channel_dispatcher::TYPE_PARALLEL },
    { "SERies", channel_dispatcher::TYPE_SERIES },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

////////////////////////////////////////////////////////////////////////////////

scpi_choice_def_t traceValueChoice[] = {
    { "VOLTage", DISPLAY_VALUE_VOLTAGE },
    { "CURRent", DISPLAY_VALUE_CURRENT },
    { "POWer", DISPLAY_VALUE_POWER },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

////////////////////////////////////////////////////////////////////////////////

static void select_channel(scpi_t * context, uint8_t ch) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    psu_context->selected_channel_index = ch;
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_instrumentSelect(scpi_t * context) {
    if (calibration::isEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS);
        return SCPI_RES_ERR;
    }

    Channel *channel = param_channel(context, TRUE);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    select_channel(context, channel->index);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_instrumentSelectQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    char buffer[256] = { 0 };
    sprintf_P(buffer, PSTR("CH%d"), (int)psu_context->selected_channel_index);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_instrumentNselect(scpi_t * context) {
    if (calibration::isEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_BAD_SEQUENCE_OF_CALIBRATION_COMMANDS);
        return SCPI_RES_ERR;
    }

    int32_t ch;
    if (!SCPI_ParamInt(context, &ch, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (!check_channel(context, ch)) {
        return SCPI_RES_ERR;
    }

    select_channel(context, ch);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_instrumentNselectQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    SCPI_ResultInt(context, psu_context->selected_channel_index);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_instrumentCoupleTracking(scpi_t * context) {
    if (channel_dispatcher::isTracked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_IN_TRACKING_MODE);
        return SCPI_RES_ERR;
    }

    int32_t type;
    if (!SCPI_ParamChoice(context, channelsCouplingChoice, &type, true)) {
        return SCPI_RES_ERR;
    }

    if (CH_NUM < 2 && type == channel_dispatcher::TYPE_NONE) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setType((channel_dispatcher::Type)type);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_instrumentCoupleTrackingQ(scpi_t * context) {
    if (channel_dispatcher::isTracked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_IN_TRACKING_MODE);
        return SCPI_RES_ERR;
    }

    char result[16];

    channel_dispatcher::Type type = channel_dispatcher::getType();
    if (type == channel_dispatcher::TYPE_PARALLEL) {
        strcpy_P(result, PSTR("PARALLEL"));
    } else if (type == channel_dispatcher::TYPE_SERIES) {
        strcpy_P(result, PSTR("SERIES"));
    } else {
        strcpy_P(result, PSTR("NONE"));
    }

    SCPI_ResultText(context, result);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_instrumentDisplayTrace(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    Channel *channel = &Channel::get(psu_context->selected_channel_index - 1);

    int32_t traceNumber;
    SCPI_CommandNumbers(context, &traceNumber, 1, 1);
    if (traceNumber < 1 || traceNumber > 2) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return SCPI_RES_ERR;
    }

    int32_t traceValue;
    if (!SCPI_ParamChoice(context, traceValueChoice, &traceValue, true)) {
        return SCPI_RES_ERR;
    }

    if (traceNumber == 1) {
        if (traceValue == channel->flags.displayValue2) {
            SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
            return SCPI_RES_ERR;
        }
        channel_dispatcher::setDisplayViewSettings(*channel, traceValue, channel->flags.displayValue2, channel->ytViewRate);
    } else {
        if (traceValue == channel->flags.displayValue1) {
            SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
            return SCPI_RES_ERR;
        }
        channel_dispatcher::setDisplayViewSettings(*channel, channel->flags.displayValue1, traceValue, channel->ytViewRate);
    }

    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_instrumentDisplayTraceQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    Channel *channel = &Channel::get(psu_context->selected_channel_index - 1);

    int32_t traceNumber;
    SCPI_CommandNumbers(context, &traceNumber, 1, 1);
    if (traceNumber < 1 || traceNumber > 2) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return SCPI_RES_ERR;
    }

    int8_t traceValue;
    if (traceNumber == 1) {
        traceValue = channel->flags.displayValue1;
    } else {
        traceValue = channel->flags.displayValue2;
    }

    char result[16];

    if (traceValue == DISPLAY_VALUE_VOLTAGE) {
        strcpy_P(result, PSTR("VOLT"));
    } else if (traceValue == DISPLAY_VALUE_CURRENT) {
        strcpy_P(result, PSTR("CURR"));
    } else {
        strcpy_P(result, PSTR("POW"));
    }

    SCPI_ResultText(context, result);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_instrumentDisplayTraceSwap(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    Channel *channel = &Channel::get(psu_context->selected_channel_index - 1);

    channel_dispatcher::setDisplayViewSettings(*channel, channel->flags.displayValue2, channel->flags.displayValue1, channel->ytViewRate);
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_instrumentDisplayYtRate(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    Channel *channel = &Channel::get(psu_context->selected_channel_index - 1);

    float ytViewRate;
    if (!get_duration_param(context, ytViewRate, GUI_YT_VIEW_RATE_MIN, GUI_YT_VIEW_RATE_MAX, GUI_YT_VIEW_RATE_DEFAULT)) {
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setDisplayViewSettings(*channel, channel->flags.displayValue1, channel->flags.displayValue2, ytViewRate);
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_instrumentDisplayYtRateQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    Channel *channel = &Channel::get(psu_context->selected_channel_index - 1);

    SCPI_ResultFloat(context, channel->ytViewRate);

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi