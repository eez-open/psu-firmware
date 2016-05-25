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
#include "scpi_mem.h"

#include "profile.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_mem_NStatesQ(scpi_t *context) {
    SCPI_ResultInt(context, NUM_PROFILE_LOCATIONS);

    return SCPI_RES_OK;
}

scpi_result_t scpi_mem_StateCatalogQ(scpi_t *context) {
    char name[PROFILE_NAME_MAX_LENGTH + 1];

    for (int i = 0; i < NUM_PROFILE_LOCATIONS; ++i) {
        profile::getName(i, name);
        SCPI_ResultText(context, name);
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_mem_StateDelete(scpi_t *context) {
    int location;
    if (!get_profile_location_param(context, location)) {
        return SCPI_RES_ERR;
    }

    if (!profile::deleteLocation(location)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_mem_StateDeleteAll(scpi_t *context) {
    if (!profile::deleteAll()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_mem_StateName(scpi_t *context) {
    int location;
    if (!get_profile_location_param(context, location)) {
        return SCPI_RES_ERR;
    }

    if (!profile::isValid(location)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    const char *name;
    size_t name_len;
    if (!SCPI_ParamCharacters(context, &name, &name_len, true)) {
        return SCPI_RES_ERR;
    }

    if (name_len > PROFILE_NAME_MAX_LENGTH) {
        SCPI_ErrorPush(context, SCPI_ERROR_TOO_MUCH_DATA);
        return SCPI_RES_ERR;
    }

    if (!profile::setName(location, name, name_len)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_mem_StateNameQ(scpi_t *context) {
    int location;
    if (!get_profile_location_param(context, location, true)) {
        return SCPI_RES_ERR;
    }

    char name[PROFILE_NAME_MAX_LENGTH + 1];
    profile::getName(location, name);

    SCPI_ResultText(context, name);

    return SCPI_RES_OK;
}

scpi_result_t scpi_mem_StateRecallAuto(scpi_t *context) {
    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (!persist_conf::enableProfileAutoRecall(enable)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_mem_StateRecallAutoQ(scpi_t *context) {
    SCPI_ResultBool(context, persist_conf::isProfileAutoRecallEnabled());

    return SCPI_RES_OK;
}

scpi_result_t scpi_mem_StateRecallSelect(scpi_t *context) {
    int location;
    if (!get_profile_location_param(context, location, true)) {
        return SCPI_RES_ERR;
    }

    if (!profile::isValid(location)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    if (!persist_conf::setProfileAutoRecallLocation(location)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_mem_StateRecallSelectQ(scpi_t *context) {
    SCPI_ResultInt(context, persist_conf::getProfileAutoRecallLocation());

    return SCPI_RES_OK;
}

scpi_result_t scpi_mem_StateValidQ(scpi_t *context) {
    int location;
    if (!get_profile_location_param(context, location, true)) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, profile::isValid(location));

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi