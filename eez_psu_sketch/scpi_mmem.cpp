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

#if OPTION_SD_CARD
#include "sd_card.h"
#endif

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

#if OPTION_SD_CARD

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

#endif

scpi_result_t scpi_cmd_mmemoryLoadList(scpi_t *context) {
#if OPTION_SD_CARD
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
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryStoreList(scpi_t *context) {
#if OPTION_SD_CARD
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
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

////////////////////////////////////////////////////////////////////////////////

#if OPTION_SD_CARD
bool getFilePath(scpi_t *context, char *filePath) {
    const char *filePathParam;
    size_t filePathParamLen;
    if (SCPI_ParamCharacters(context, &filePathParam, &filePathParamLen, false)) {
        if (filePathParamLen > MAX_PATH_LENGTH) {
            SCPI_ErrorPush(context, SCPI_ERROR_CHARACTER_DATA_TOO_LONG);
            return false;
        }
        strncpy(filePath, filePathParam, filePathParamLen);
        filePath[filePathParamLen] = 0;
    } else {
        if (SCPI_ParamErrorOccurred(context)) {
            return false;
        }

        strcpy(filePath, "/");
    }

    util::replaceCharacter(filePath, '\\', '/');

    return true;
}

void catalogCallback(void *param, const char *name, sd_card::FileType type, size_t size) {
    scpi_t *context = (scpi_t *)param;

    char buffer[MAX_PATH_LENGTH + 6 + 10 + 1];

    // max. MAX_PATH_LENGTH characters
    size_t nameLength = strlen(name);
    size_t position;
    if (nameLength > MAX_PATH_LENGTH) {
        strncpy(buffer, name, MAX_PATH_LENGTH);
        position = MAX_PATH_LENGTH;
    } else {
        strcpy(buffer, name);
        position = nameLength;
    }
    
    // max. 6 characters
    switch (type) {
    case sd_card::FILE_TYPE_BIN:
        strcpy(buffer + position, ",BIN,");
        position += 5;
        break;
    
    default:
        strcpy(buffer + position, ",FOLD,");
        position += 6;
        break;
    }

    // max. 10 characters (for 4294967296)
    sprintf(buffer + position, "%lu", (unsigned long)size);

    SCPI_ResultText(context, buffer);

    psu::tick();
}
#endif

scpi_result_t scpi_cmd_mmemoryCatalogQ(scpi_t *context) {
#if OPTION_SD_CARD
    char dirPath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, dirPath)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!sd_card::catalog(dirPath, context, catalogCallback, &err)) {
        if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

#if OPTION_SD_CARD
void uploadCallback(void *param, const void *buffer, size_t size) {
    if (buffer == NULL && size == 0) {
        return;
    }

    scpi_t *context = (scpi_t *)param;

    if (buffer == NULL && size > 0) {
        SCPI_ResultArbitraryBlockHeader(context, size);
        return;
    }

    SCPI_ResultArbitraryBlockData(context, buffer, size);

    psu::tick();
}
#endif

scpi_result_t scpi_cmd_mmemoryUploadQ(scpi_t *context) {
#if OPTION_SD_CARD
    char filePath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, filePath)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!sd_card::upload(filePath, context, uploadCallback, &err)) {
        if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}


scpi_result_t scpi_cmd_mmemoryDownloadFname(scpi_t *context) {
#if OPTION_SD_CARD
    scpi_psu_t *psuContext = (scpi_psu_t *)context->user_context;

    if (!getFilePath(context, psuContext->filePath)) {
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryDownloadData(scpi_t *context) {
#if OPTION_SD_CARD
    scpi_psu_t *psuContext = (scpi_psu_t *)context->user_context;

    if (psuContext->filePath[0] == 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_FILE_NAME_ERROR);
        return SCPI_RES_ERR;
    }

    const char *buffer;
    size_t size;
    if (!SCPI_ParamArbitraryBlock(context, &buffer, &size, true)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!sd_card::download(psuContext->filePath, buffer, size, &err)) {
        if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryDelete(scpi_t *context) {
#if OPTION_SD_CARD
    char filePath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, filePath)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!sd_card::deleteFile(filePath, &err)) {
        if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

}
}
} // namespace eez::psu::scpi 