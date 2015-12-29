/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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
#include <scpi-parser.h>
#include "scpi_psu.h"
#include "scpi_core.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_meas_CurrentQ(scpi_t * context) {
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    char buffer[256] = { 0 };
    util::strcatFloat(buffer, channel->i.mon);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

scpi_result_t scpi_meas_PowerQ(scpi_t * context) {
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    char buffer[256] = { 0 };
    util::strcatFloat(buffer, channel->u.mon * channel->i.mon);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

scpi_result_t scpi_meas_VoltageQ(scpi_t * context) {
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    char buffer[256] = { 0 };
    util::strcatFloat(buffer, channel->u.mon);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

scpi_result_t scpi_meas_TemperatureQ(scpi_t * context) {
    int32_t sensor;
    if (!SCPI_ParamChoice(context, all_temp_sensor_choice, &sensor, FALSE)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
    }

    char buffer[256] = { 0 };
    util::strcatFloat(buffer, temperature::measure((temp_sensor::Type)sensor));
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi