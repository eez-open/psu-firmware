/*
 * EEZ PSU Firmware
 * Copyright (C) 2017-present, Envox d.o.o.
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

#include "eez/psu/psu.h"
#include "eez/psu/scpi/psu.h"
#include "eez/psu/profile.h"
#include "eez/psu/channel_dispatcher.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_senseCurrentDcRangeUpper(scpi_t * context) {
    CurrentRangeSelectionMode mode;

    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return SCPI_RES_ERR;
    }
    if (param.special) {
        if (param.tag == SCPI_NUM_MIN) {
            mode = CURRENT_RANGE_SELECTION_ALWAYS_LOW;
        }
        else if (param.tag == SCPI_NUM_MAX) {
            mode = CURRENT_RANGE_SELECTION_ALWAYS_HIGH;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            mode = CURRENT_RANGE_SELECTION_USE_BOTH;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return SCPI_RES_ERR;
        }
    } else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_AMPER) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return SCPI_RES_ERR;
        }

        float value = (float)param.value;
        if (mw::equal(value, 0.5f, getPrecision(UNIT_AMPER))) {
            mode = CURRENT_RANGE_SELECTION_ALWAYS_LOW;
        } else if (mw::equal(value, 5.0f, getPrecision(UNIT_AMPER))) {
            mode = CURRENT_RANGE_SELECTION_ALWAYS_HIGH;
        } else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return SCPI_RES_ERR;
        }
    }

    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!channel->hasSupportForCurrentDualRange()) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        return SCPI_RES_ERR;
    }

    if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    channel->setCurrentRangeSelectionMode(mode);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_senseCurrentDcRangeUpperQ(scpi_t *context) {
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!channel->hasSupportForCurrentDualRange()) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        return SCPI_RES_ERR;
    }

    if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    CurrentRangeSelectionMode mode = channel->getCurrentRangeSelectionMode();

    if (mode == CURRENT_RANGE_SELECTION_ALWAYS_LOW) {
        SCPI_ResultFloat(context, 0.5);
    } else if (mode == CURRENT_RANGE_SELECTION_ALWAYS_HIGH) {
        SCPI_ResultFloat(context, 5);
    } else {
        SCPI_ResultText(context, "Default");
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_senseCurrentDcRangeAuto(scpi_t *context) {
    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!channel->hasSupportForCurrentDualRange()) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        return SCPI_RES_ERR;
    }

    if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    channel->enableAutoSelectCurrentRange(enable);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_senseCurrentDcRangeAutoQ(scpi_t * context) {
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!channel->hasSupportForCurrentDualRange()) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        return SCPI_RES_ERR;
    }

    if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }
    SCPI_ResultBool(context, channel->isAutoSelectCurrentRangeEnabled());

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi