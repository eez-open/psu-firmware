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

#include "eeprom.h"
#include "profile.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_coreCls(scpi_t * context) {
    return SCPI_CoreCls(context);
}

scpi_result_t scpi_cmd_coreEse(scpi_t * context) {
    return SCPI_CoreEse(context);
}

scpi_result_t scpi_cmd_coreEseQ(scpi_t * context) {
    return SCPI_CoreEseQ(context);
}

scpi_result_t scpi_cmd_coreEsrQ(scpi_t * context) {
    return SCPI_CoreEsrQ(context);
}

scpi_result_t scpi_cmd_coreIdnQ(scpi_t * context) {
    return SCPI_CoreIdnQ(context);
}

scpi_result_t scpi_cmd_coreOpc(scpi_t * context) {
    return SCPI_CoreOpc(context);
}

scpi_result_t scpi_cmd_coreOpcQ(scpi_t * context) {
    return SCPI_CoreOpcQ(context);
}

/**
* Implement IEEE488.2 *RCL
*
* Recalls the PSU state stored in the specified storage location.
*
* Return SCPI_RES_OK
*/
scpi_result_t scpi_cmd_coreRcl(scpi_t * context) {
    int location;
    if (!get_profile_location_param(context, location)) {
        return SCPI_RES_ERR;
    }
    
    if (!profile::recall(location)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_coreRst(scpi_t * context) {
    return SCPI_CoreRst(context);
}

/**
* Implement IEEE488.2 *SAV
*
* Stores the current PSU state in the specified storage location
*
* Return SCPI_RES_OK
*/
scpi_result_t scpi_cmd_coreSav(scpi_t * context) {
    int location;
    if (!get_profile_location_param(context, location)) {
        return SCPI_RES_ERR;
    }

    if (!profile::saveAtLocation(location)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXTERNAL_EEPROM_SAVE_FAILED);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_coreSre(scpi_t * context) {
    return SCPI_CoreSre(context);
}

scpi_result_t scpi_cmd_coreSreQ(scpi_t * context) {
    return SCPI_CoreSreQ(context);
}

scpi_result_t scpi_cmd_coreStbQ(scpi_t * context) {
    return SCPI_CoreStbQ(context);
}

/**
* Implement IEEE488.2 *TST?
*
* Result should be 0 if everything is ok
* Result should be 1 if something goes wrong
*
* Return SCPI_RES_OK
*/
scpi_result_t scpi_cmd_coreTstQ(scpi_t * context) {
    SCPI_ResultInt(context, psu::test() ? 0 : 1);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_coreWai(scpi_t * context) {
    return SCPI_CoreWai(context);
}


}
}
} // namespace eez::psu::scpi