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
#include "scpi_appl.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

static scpi_choice_def_t current_or_voltage_choice[] = {
    { "CURRent", 0 },
    { "VOLTage", 1 },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_appl_Apply(scpi_t *context) {
    Channel *channel = param_channel(context, TRUE);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float voltage;
    if (!get_voltage_param(context, voltage, channel, 0)) {
        return SCPI_RES_ERR;
    }

    bool call_set_current = false;
    float current;

    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        // no CURRent parameter
    }
    else {
        if (!get_current_from_param(context, param, current, channel, 0)) {
            return SCPI_RES_ERR;
        }
        call_set_current = true;
    }

	if (voltage > channel->getVoltageLimit()) {
        SCPI_ErrorPush(context, SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED);
        return SCPI_RES_ERR;
	}

	if (call_set_current && current > channel->getCurrentLimit()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CURRENT_LIMIT_EXCEEDED);
        return SCPI_RES_ERR;
	}

    if (voltage * (call_set_current ? current : channel->i.set) > channel->getPowerLimit()) {
        SCPI_ErrorPush(context, SCPI_ERROR_POWER_LIMIT_EXCEEDED);
        return SCPI_RES_ERR;
    }

    // set voltage
    channel->setVoltage(voltage);

    // set current
    if (call_set_current) {
        channel->setCurrent(current);
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_appl_ApplyQ(scpi_t * context) {
    Channel *channel = param_channel(context, TRUE);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    char buffer[256] = { 0 };

    int32_t current_or_voltage;
    if (!SCPI_ParamChoice(context, current_or_voltage_choice, &current_or_voltage, FALSE)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }

        // return both current and voltage
        sprintf_P(buffer, PSTR("CH%d:"), channel->index);
        util::strcatVoltage(buffer, channel->U_MAX);
        strcat(buffer, "/");
        util::strcatCurrent(buffer, channel->I_MAX);
        strcat(buffer, ", ");

        util::strcatFloat(buffer, channel->u.set);
        strcat(buffer, ", ");
        util::strcatFloat(buffer, channel->i.set);
    }
    else {
        if (current_or_voltage == 0) {
            // return only current
            util::strcatFloat(buffer, channel->i.set);
        }
        else {
            // return only voltage
            util::strcatFloat(buffer, channel->u.set);
        }
    }

    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi