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

#include "trigger.h"
#include "list.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

bool getFileNameParam(scpi_t *context, char *filePath, int *err) {
    const char *param;
    size_t len;

    if (!SCPI_ParamCharacters(context, &param, &len, true)) {
        *err = 0;
        return false;
    }

    if (len > 8) {
        *err = SCPI_ERROR_CHARACTER_DATA_TOO_LONG;
        return false;
    }

    char fileName[8 + 1];
    strncpy(fileName, param, len);
    fileName[len] = 0;

    strcpy(filePath, LISTS_DIR);
    strcat(filePath, PATH_SEPARATOR);
    strcat(filePath, fileName);
    strcat(filePath, LIST_FILE_EXTENSION);

    return true;
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_mmemoryLoadList(scpi_t *context) {
	Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!trigger::isIdle()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
        return SCPI_RES_ERR;
    }

    char filePath[MAX_PATH_LENGTH];
    int err;
    if (!getFileNameParam(context, filePath, &err)) {
        if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
        return SCPI_RES_ERR;
    }

    if (!list::loadList(*channel, filePath, &err)) {
        SCPI_ErrorPush(context, err);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_mmemoryStoreList(scpi_t *context) {
	Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!list::areListLengthsEquivalent(*channel)) {
        SCPI_ErrorPush(context, SCPI_ERROR_LIST_LENGTHS_NOT_EQUIVALENT);
        return SCPI_RES_ERR;
    }

    char filePath[MAX_PATH_LENGTH];
    int err;
    if (!getFileNameParam(context, filePath, &err)) {
        if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
        return SCPI_RES_ERR;
    }

    if (!list::saveList(*channel, filePath, &err)) {
        SCPI_ErrorPush(context, err);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi