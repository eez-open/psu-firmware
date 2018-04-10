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

#if OPTION_DISPLAY
#include "gui.h"
#endif

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

#if OPTION_SD_CARD

void addExtension(char *filePath, const char *ext) {
    if (!util::endsWith(filePath, ext)) {
        strcat(filePath, ext);
    }
}

#endif

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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif
}

////////////////////////////////////////////////////////////////////////////////

void catalogCallback(void *param, const char *name, const char *type, size_t size) {
    scpi_t *context = (scpi_t *)param;

    char buffer[MAX_PATH_LENGTH + 10 + 10 + 1];

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
    
    // max. 10 characters
    buffer[position++] = ',';
    strcpy(buffer + position, type);
    position += strlen(type);
    buffer[position++] = ',';

    // max. 10 characters (for 4294967296)
    sprintf(buffer + position, "%lu", (unsigned long)size);

    SCPI_ResultText(context, buffer);

    psu::tick();
}


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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryInformationQ(scpi_t *context) {
#if OPTION_SD_CARD
    uint64_t usedSpace, freeSpace;
    if (!sd_card::getInfo(usedSpace, freeSpace)) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt64(context, usedSpace);
    SCPI_ResultUInt64(context, freeSpace);

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif
}

////////////////////////////////////////////////////////////////////////////////

#if OPTION_SD_CARD
void uploadCallback(void *param, const void *buffer, size_t size) {
    if (buffer == NULL && size == -1) {
        return;
    }

    scpi_t *context = (scpi_t *)param;

    if (buffer == NULL) {
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
		if (err != SCPI_ERROR_FILE_TRANSFER_ABORTED) {
			event_queue::pushEvent(event_queue::EVENT_ERROR_FILE_UPLOAD_FAILED);
		}

		if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
        
		return SCPI_RES_ERR;
    }

	event_queue::pushEvent(event_queue::EVENT_INFO_FILE_UPLOAD_SUCCEEDED);

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif
}

////////////////////////////////////////////////////////////////////////////////

#if OPTION_SD_CARD
static char g_downloadFilePath[MAX_PATH_LENGTH + 1];
static uint32_t g_downloadSize;
static bool g_downloading;
static bool g_aborted;
static uint32_t g_downloaded;

void abortDownloading();

void startDownloading() {
	g_downloading = true;
	g_downloaded = 0;
#if OPTION_DISPLAY
	gui::showProgressPage("Downloading...", abortDownloading);
#endif
}

void finishDownloading(int16_t eventId) {
	if (eventId != event_queue::EVENT_INFO_FILE_DOWNLOAD_SUCCEEDED) {
		sd_card::deleteFile(g_downloadFilePath, 0);
	}
	event_queue::pushEvent(eventId);
#if OPTION_DISPLAY
	gui::hideProgressPage();
#endif
	g_downloading = false;
	g_downloadFilePath[0] = 0;
}

void abortDownloading() {
	finishDownloading(event_queue::EVENT_WARNING_FILE_DOWNLOAD_ABORTED);
	g_aborted = true;
}
#endif

scpi_result_t scpi_cmd_mmemoryDownloadFname(scpi_t *context) {
#if OPTION_SD_CARD
	if (!getFilePath(context, g_downloadFilePath, true)) {
		return SCPI_RES_ERR;
	}

	if (g_downloading) {
		finishDownloading(event_queue::EVENT_INFO_FILE_DOWNLOAD_SUCCEEDED);
		return SCPI_RES_OK;
	}

	g_downloadSize = 0;
	g_aborted = false;

	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryDownloadSize(scpi_t *context) {
#if OPTION_SD_CARD
	uint32_t size;
	if (!SCPI_ParamUInt32(context, &size, true)) {
		return SCPI_RES_ERR;
	}

	if (size > 2147483648L) {
		SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
		return SCPI_RES_ERR;
	}
	
	g_downloadSize = size;

	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
	return SCPI_RES_ERR;
#endif
}


scpi_result_t scpi_cmd_mmemoryDownloadData(scpi_t *context) {
#if OPTION_SD_CARD
    if (persist_conf::isSdLocked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_MEDIA_PROTECTED);
        return SCPI_RES_ERR;
    }

	if (g_aborted) {
		SCPI_ErrorPush(context, SCPI_ERROR_FILE_TRANSFER_ABORTED);
		return SCPI_RES_ERR;
	}

    if (g_downloadFilePath[0] == 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_FILE_NAME_ERROR);
        return SCPI_RES_ERR;
    }

    const char *buffer;
    size_t size;
    if (!SCPI_ParamArbitraryBlock(context, &buffer, &size, true)) {
        return SCPI_RES_ERR;
    }

	bool downloading = g_downloading;
	if (!downloading) {
		startDownloading();
	}

    int err;
    if (!sd_card::download(g_downloadFilePath, !downloading, buffer, size, &err)) {
		finishDownloading(event_queue::EVENT_ERROR_FILE_DOWNLOAD_FAILED);
        if (err != 0) {
            SCPI_ErrorPush(context, err);
        }
		return SCPI_RES_ERR;
    }

#if OPTION_DISPLAY
	g_downloaded += size;
	gui::updateProgressPage(g_downloaded, g_downloadSize);
#endif

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryDownloadAbort(scpi_t *context) {
#if OPTION_SD_CARD
	abortDownloading();
	return SCPI_RES_OK;
#else
	SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_mmemoryLock(scpi_t *context) {
#if OPTION_SD_CARD
    if (!checkPassword(context, persist_conf::devConf2.systemPassword)) {
        return SCPI_RES_ERR;
    }

    persist_conf::setSdLocked(true);

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryLockQ(scpi_t *context) {
#if OPTION_SD_CARD
    SCPI_ResultBool(context, persist_conf::isSdLocked());
    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryUnlock(scpi_t *context) {
#if OPTION_SD_CARD
    if (!checkPassword(context, persist_conf::devConf2.systemPassword)) {
        return SCPI_RES_ERR;
    }

    persist_conf::setSdLocked(false);

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
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

    char filePath[MAX_PATH_LENGTH + sizeof(list::LIST_EXT) + 1];
    if (!getFilePath(context, filePath, true)) {
        return SCPI_RES_ERR;
    }

    addExtension(filePath, list::LIST_EXT);

    int err;
    if (!list::saveList(*channel, filePath, &err)) {
        SCPI_ErrorPush(context, err);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_mmemoryLoadProfile(scpi_t *context) {
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
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_mmemoryStoreProfile(scpi_t *context) {
#if OPTION_SD_CARD
    if (persist_conf::isSdLocked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_MEDIA_PROTECTED);
        return SCPI_RES_ERR;
    }

    char filePath[MAX_PATH_LENGTH + sizeof(profile::PROFILE_EXT) + 1];
    if (!getFilePath(context, filePath, true)) {
        return SCPI_RES_ERR;
    }

    addExtension(filePath, profile::PROFILE_EXT);

    int err;
    if (!profile::saveToFile(filePath, &err)) {
        SCPI_ErrorPush(context, err);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif
}

}
}
} // namespace eez::psu::scpi 