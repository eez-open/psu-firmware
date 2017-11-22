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
#include "profile.h"

#if OPTION_SD_CARD
#include "sd_card.h"
#endif

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

#if OPTION_SD_CARD
void cleanupPath(char *filePath) {
    util::replaceCharacter(filePath, '\\', '/');

    char *q = filePath;

    for (char *p = filePath; *p; ++p) {
        if (*p == '\\') {
            *p = '/';
        }

        if (*p == '/' && (p > filePath && *(p - 1) == '/')) {
            // '//' -> '/'
            continue;
        } else if (*p == '.') {
            if (!*(p + 1)) {
                // '<...>/.' -> '<...>'
                break;
            } else if (*(p + 1) == '/') {
                // '<...>/./<...>' -> '<...>/<...>'
                ++p;
                continue;
            } else if (*(p + 1) == '.') {
                // '<...>/something/..<...>' -> '<...>/<...>'
                q -= 2;
                while (true) {
                    if (q < filePath) {
                        q = filePath;
                        break;
                    }
                    if (*q == '/') {
                        break;
                    }
                    --q;
                }
                ++p;
                continue;
            }
        }

        *q++ = *p;
    }

    // remove trailing '/'
    if (q > filePath && *(q - 1) == '/') {
        --q;
    }

    // if empty then make it '/'
    if (q == filePath) {
        *q++ = '/';
    }
    
    *q = 0;
}

bool getFilePath(scpi_t *context, char *filePath, bool mandatory) {
    scpi_psu_t *psuContext = (scpi_psu_t *)context->user_context;

    const char *filePathParam;
    size_t filePathParamLen;
    if (SCPI_ParamCharacters(context, &filePathParam, &filePathParamLen, mandatory)) {
        if (filePathParamLen > MAX_PATH_LENGTH) {
            SCPI_ErrorPush(context, SCPI_ERROR_CHARACTER_DATA_TOO_LONG);
            return false;
        }

        // is it absolute file path?
        if (filePathParam[0] == '/' || filePathParam[0] == '\\') {
            // yes
            strncpy(filePath, filePathParam, filePathParamLen);
            filePath[filePathParamLen] = 0;
        } else {
            // no, combine with current directory to get absolute path
            size_t currentDirectoryLen = strlen(psuContext->currentDirectory);
            size_t filePathLen = currentDirectoryLen + 1 + filePathParamLen;
            if (filePathLen > MAX_PATH_LENGTH) {
                SCPI_ErrorPush(context, SCPI_ERROR_CHARACTER_DATA_TOO_LONG);
                return false;
            }
            strncpy(filePath, psuContext->currentDirectory, currentDirectoryLen);
            filePath[currentDirectoryLen] = '/';
            strncpy(filePath + currentDirectoryLen + 1, filePathParam, filePathParamLen);
            filePath[filePathLen] = 0;
        }
    } else {
        if (SCPI_ParamErrorOccurred(context)) {
            return false;
        }
        strcpy(filePath, psuContext->currentDirectory);
    }

    cleanupPath(filePath);

    return true;
}

scpi_result_t scpi_cmd_mmemoryCdirectory(scpi_t *context) {
#if OPTION_SD_CARD
    char dirPath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, dirPath, true)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!sd_card::exists(dirPath, &err)) {
        if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
        return SCPI_RES_ERR;
    }

    scpi_psu_t *psuContext = (scpi_psu_t *)context->user_context;
    strcpy(psuContext->currentDirectory, dirPath);

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryCdirectoryQ(scpi_t *context) {
#if OPTION_SD_CARD
    scpi_psu_t *psuContext = (scpi_psu_t *)context->user_context;
    if (psuContext->currentDirectory[0]) {
        SCPI_ResultText(context, psuContext->currentDirectory);
    } else {
        SCPI_ResultText(context, "/");
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

////////////////////////////////////////////////////////////////////////////////

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
    if (!getFilePath(context, dirPath, false)) {
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

scpi_result_t scpi_cmd_mmemoryCatalogLengthQ(scpi_t *context) {
#if OPTION_SD_CARD
    char dirPath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, dirPath, false)) {
        return SCPI_RES_ERR;
    }

    size_t length;
    int err;
    if (!sd_card::catalogLength(dirPath, &length, &err)) {
        if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32(context, (uint32_t)length);

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

////////////////////////////////////////////////////////////////////////////////

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
    if (!getFilePath(context, filePath, true)) {
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

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_mmemoryDownloadFname(scpi_t *context) {
#if OPTION_SD_CARD
    scpi_psu_t *psuContext = (scpi_psu_t *)context->user_context;

    if (!getFilePath(context, psuContext->downloadFilePath, true)) {
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
    if (persist_conf::isSdLocked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_MEDIA_PROTECTED);
        return SCPI_RES_ERR;
    }

    scpi_psu_t *psuContext = (scpi_psu_t *)context->user_context;

    if (psuContext->downloadFilePath[0] == 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_FILE_NAME_ERROR);
        return SCPI_RES_ERR;
    }

    const char *buffer;
    size_t size;
    if (!SCPI_ParamArbitraryBlock(context, &buffer, &size, true)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!sd_card::download(psuContext->downloadFilePath, buffer, size, &err)) {
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

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_mmemoryMove(scpi_t *context) {
#if OPTION_SD_CARD
    if (persist_conf::isSdLocked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_MEDIA_PROTECTED);
        return SCPI_RES_ERR;
    }

    char sourcePath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, sourcePath, true)) {
        return SCPI_RES_ERR;
    }

    char destinationPath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, destinationPath, true)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!sd_card::moveFile(sourcePath, destinationPath, &err)) {
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

scpi_result_t scpi_cmd_mmemoryCopy(scpi_t *context) {
#if OPTION_SD_CARD
    if (persist_conf::isSdLocked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_MEDIA_PROTECTED);
        return SCPI_RES_ERR;
    }

    char sourcePath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, sourcePath, true)) {
        return SCPI_RES_ERR;
    }

    char destinationPath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, destinationPath, true)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!sd_card::copyFile(sourcePath, destinationPath, &err)) {
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
    if (persist_conf::isSdLocked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_MEDIA_PROTECTED);
        return SCPI_RES_ERR;
    }

    char filePath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, filePath, true)) {
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

scpi_result_t scpi_cmd_mmemoryMdirectory(scpi_t *context) {
#if OPTION_SD_CARD
    if (persist_conf::isSdLocked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_MEDIA_PROTECTED);
        return SCPI_RES_ERR;
    }

    char dirPath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, dirPath, false)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!sd_card::makeDir(dirPath, &err)) {
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

scpi_result_t scpi_cmd_mmemoryRdirectory(scpi_t *context) {
#if OPTION_SD_CARD
    if (persist_conf::isSdLocked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_MEDIA_PROTECTED);
        return SCPI_RES_ERR;
    }

    char dirPath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, dirPath, false)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!sd_card::removeDir(dirPath, &err)) {
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

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_mmemoryDateQ(scpi_t *context) {
#if OPTION_SD_CARD
    char filePath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, filePath, true)) {
        return SCPI_RES_ERR;
    }

    uint8_t year, month, day;
    int err;
    if (!sd_card::getDate(filePath, year, month, day, &err)) {
        if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
        return SCPI_RES_ERR;
    }

    char buffer[16] = { 0 };
    sprintf_P(buffer, PSTR("%d, %d, %d"), (int)(year + 2000), (int)month, (int)day);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryTimeQ(scpi_t *context) {
#if OPTION_SD_CARD
    char filePath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, filePath, true)) {
        return SCPI_RES_ERR;
    }

    uint8_t hour, minute, second;
    int err;
    if (!sd_card::getTime(filePath, hour, minute, second, &err)) {
        if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
        return SCPI_RES_ERR;
    }

    char buffer[16] = { 0 };
    sprintf_P(buffer, PSTR("%d, %d, %d"), (int)hour, (int)minute, (int)second);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_mmemoryLock(scpi_t *context) {
#if OPTION_SD_CARD
    persist_conf::setSdLocked(true);
    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryLockQ(scpi_t *context) {
#if OPTION_SD_CARD
    SCPI_ResultBool(context, persist_conf::isSdLocked());
    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryUnlock(scpi_t *context) {
#if OPTION_SD_CARD
    persist_conf::setSdLocked(false);
    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

////////////////////////////////////////////////////////////////////////////////

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

    char filePath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, filePath, true)) {
        return SCPI_RES_ERR;
    }

    int err;
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
    if (persist_conf::isSdLocked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_MEDIA_PROTECTED);
        return SCPI_RES_ERR;
    }

    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!list::areListLengthsEquivalent(*channel)) {
        SCPI_ErrorPush(context, SCPI_ERROR_LIST_LENGTHS_NOT_EQUIVALENT);
        return SCPI_RES_ERR;
    }

    char filePath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, filePath, true)) {
        return SCPI_RES_ERR;
    }

    int err;
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

scpi_result_t scpi_cmd_mmemoryLoadState(scpi_t *context) {
#if OPTION_SD_CARD
    char filePath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, filePath, true)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!profile::recallFromFile(filePath, &err)) {
        SCPI_ErrorPush(context, err);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryStoreState(scpi_t *context) {
#if OPTION_SD_CARD
    if (persist_conf::isSdLocked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_MEDIA_PROTECTED);
        return SCPI_RES_ERR;
    }

    char filePath[MAX_PATH_LENGTH + 1];
    if (!getFilePath(context, filePath, true)) {
        return SCPI_RES_ERR;
    }

    int err;
    if (!profile::saveToFile(filePath, &err)) {
        SCPI_ErrorPush(context, err);
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