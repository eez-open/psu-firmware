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
#include "temperature.h"
#include "channel_dispatcher.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_measureScalarCurrentDcQ(scpi_t * context) {
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    char buffer[256] = { 0 };
    util::strcatFloat(buffer, channel_dispatcher::getIMon(*channel), VALUE_TYPE_FLOAT_AMPER, channel->index-1);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_measureScalarPowerDcQ(scpi_t * context) {
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    char buffer[256] = { 0 };
    util::strcatFloat(buffer, channel_dispatcher::getUMon(*channel) * channel_dispatcher::getIMon(*channel), getNumSignificantDecimalDigits(VALUE_TYPE_FLOAT_WATT));
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_measureScalarVoltageDcQ(scpi_t * context) {
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    char buffer[256] = { 0 };
    util::strcatFloat(buffer, channel_dispatcher::getUMon(*channel), getNumSignificantDecimalDigits(VALUE_TYPE_FLOAT_VOLT));
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_measureScalarTemperatureThermistorDcQ(scpi_t * context) {
    int32_t sensor;
    if (!param_temp_sensor(context, sensor)) {
		return SCPI_RES_ERR;
    }

    char buffer[256] = { 0 };
    util::strcatFloat(buffer, temperature::sensors[sensor].measure(), getNumSignificantDecimalDigits(VALUE_TYPE_FLOAT_CELSIUS));
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi