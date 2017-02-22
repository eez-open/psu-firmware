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
 
#include "psu.h"
#include "scpi_psu.h"

#include "persist_conf.h"

namespace eez {
namespace psu {
namespace scpi {

scpi_result_t scpi_cmd_displayBrightness(scpi_t *context) {
    return SCPI_RES_ERR;
}

scpi_result_t scpi_cmd_displayBrightnessQ(scpi_t *context) {
    return SCPI_RES_ERR;
}

scpi_result_t scpi_cmd_displayWindowState(scpi_t *context) {
    bool onOff;
    if (!SCPI_ParamBool(context, &onOff, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (!persist_conf::setDisplayState(onOff ? 1 : 0)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_displayWindowStateQ(scpi_t *context) {
    SCPI_ResultBool(context, persist_conf::devConf2.flags.displayState);
    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi