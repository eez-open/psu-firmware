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

#if OPTION_SD_CARD

#include "sd_card.h"
#include "datetime.h"

#include "list.h"
#include "profile.h"

#if OPTION_DISPLAY
#include "gui.h"
#endif

SdFat SD;

namespace eez {
namespace psu {
namespace sd_card {

TestResult g_testResult = TEST_FAILED;

////////////////////////////////////////////////////////////////////////////////

void dateTime(uint16_t* date, uint16_t* time) {
    int year, month, day, hour, minute, second;
    datetime::breakTime(datetime::nowUtc(), year, month, day, hour, minute, second);
    *date = FAT_DATE(year, month, day);
    *time = FAT_TIME(hour, minute, second);
}

#ifndef EEZ_PSU_SIMULATOR
bool g_cardBeginResult;
int g_cardBeginErrorCode;
bool g_fsBeginResult;
int g_fsBeginErrorCode;
#endif

void init() {
	bool initResult;

#ifdef EEZ_PSU_SIMULATOR
	initResult = SD.begin(LCDSD_CS, SPI_HALF_SPEED);
#else
	g_cardBeginResult = SD.cardBegin(LCDSD_CS, SPI_HALF_SPEED);
	if (g_cardBeginResult) {
		g_fsBeginResult = SD.fsBegin();
		if (!g_fsBeginResult) {
			g_fsBeginErrorCode = SD.fsBeginErrorCode();
		}
	} else {
		g_cardBeginErrorCode = SD.cardErrorCode();
	}
	initResult = g_cardBeginResult && g_fsBeginResult;
#endif

    if (!initResult) {
        g_testResult = TEST_FAILED;
    } else {
#ifdef EEZ_PSU_SIMULATOR
        makeParentDir("/");
#endif
        g_testResult = TEST_OK;

        SdFile::dateTimeCallback(dateTime);
    }
}

bool test() {
    return g_testResult != TEST_FAILED;
}

void dumpInfo(char *buffer) {
#ifndef EEZ_PSU_SIMULATOR
	FatVolume* vol = SD.vol();
	
	sprintf(buffer + strlen(buffer), "SD volume is FAT %d\n", int(vol->fatType()));

	sprintf(buffer + strlen(buffer), "SD card begin result: %d\n", int(g_cardBeginResult));
	if (!g_cardBeginResult) {
		sprintf(buffer + strlen(buffer), "SD card begin result error: %d\n", g_cardBeginErrorCode);
	}
	sprintf(buffer + strlen(buffer), "SD fs begin result: %d\n", int(g_fsBeginResult));
	if (!g_fsBeginResult) {
		sprintf(buffer + strlen(buffer), "SD fs begin result error: %d\n", g_fsBeginErrorCode);
	}

	sprintf(buffer + strlen(buffer), "SD blocks per cluster: %d\n", int(vol->blocksPerCluster()));
	sprintf(buffer + strlen(buffer), "SD cluster count: %d\n", int(vol->clusterCount()));
	sprintf(buffer + strlen(buffer), "SD fat start block: %d\n", int(vol->fatStartBlock()));
	sprintf(buffer + strlen(buffer), "SD fat count: %d\n", int(vol->fatCount()));
	sprintf(buffer + strlen(buffer), "SD blocks per fat: %d\n", int(vol->blocksPerFat()));
	sprintf(buffer + strlen(buffer), "SD root dir start: %d\n", int(vol->rootDirStart()));
	sprintf(buffer + strlen(buffer), "SD data start block: %d\n", int(vol->dataStartBlock()));

	csd_t csd;
	if (!SD.card()->readCSD(&csd)) {
		sprintf(buffer + strlen(buffer), "SD readCSD failed\n");
		return;
	}

	uint8_t eraseSingleBlock;
	uint32_t eraseSize;

	if (csd.v1.csd_ver == 0) {
		eraseSingleBlock = csd.v1.erase_blk_en;
		eraseSize = (csd.v1.sector_size_high << 1) | csd.v1.sector_size_low;
	} else if (csd.v2.csd_ver == 1) {
		eraseSingleBlock = csd.v2.erase_blk_en;
		eraseSize = (csd.v2.sector_size_high << 1) | csd.v2.sector_size_low;
	} else {
		sprintf(buffer + strlen(buffer), "SD csd version error\n");
		return;
	}
	eraseSize++;

	uint32_t cardSize = SD.card()->cardSize();
	sprintf(buffer + strlen(buffer), "SD card size: %d MB\n", int(0.000512 * cardSize));

	sprintf(buffer + strlen(buffer), "SD flash erase size: %d blocks\n", int(eraseSize));

	if (eraseSingleBlock) {
		sprintf(buffer + strlen(buffer), "SD erase single block\n");
	}

	if (vol->dataStartBlock() % eraseSize) {
		sprintf(buffer + strlen(buffer), "SD data area is not aligned on flash erase boundaries! Download and use formatter from www.sdcard.org!\n");
	}
#endif
}

#ifndef isSpace
bool isSpace(int c) {
    return c == '\r' || c == '\n' || c == '\t' || c == ' ';
}
#endif

void matchZeroOrMoreSpaces(File &file) {
    while (true) {
        int c = file.peek();
        if (!isSpace(c)) {
            return;
        }
        file.read();
    }
}

bool match(File& file, char c) {
    matchZeroOrMoreSpaces(file);
    if (file.peek() == c) {
        file.read();
        return true;
    }
    return false;
}

bool match(File& file, float &result) {
    matchZeroOrMoreSpaces(file);

    int c = file.peek();
    if (c == -1) {
        return false;
    }

    bool isNegative;
    if (c == '-') {
        file.read();
        isNegative = true;
        c = file.peek();
    } else {
        isNegative = false;
    }

    bool isFraction = false;
    float fraction = 1.0;

    long value = -1;

    while (true) {
        if (c == '.') {
            if (isFraction) {
                return false;
            }
            isFraction = true;
        } else if (c >= '0' && c <= '9') {
            if (value == -1) {
                value = 0;
            }

            value = value * 10 + c - '0';

            if (isFraction) {
                fraction *= 0.1f;
            }
        } else {
            if (value == -1) {
                return false;
            }

            result = (float)value;
            if (isNegative) {
                result = -result;
            }
            if (isFraction) {
                result *= fraction;
            }

            return true;
        }

        file.read();
        c = file.peek();
   }
}

bool makeParentDir(const char *filePath) {
    char dirPath[MAX_PATH_LENGTH];
    util::getParentDir(filePath, dirPath);
    return SD.mkdir(dirPath);
}

bool exists(const char *dirPath, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    if (!SD.exists(dirPath)) {
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    return true;
}

bool catalog(const char *dirPath, void *param, void (*callback)(void *param, const char *name, const char *type, size_t size), int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    File dir = SD.open(dirPath);
    if (!dir) {
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    dir.rewindDirectory();

    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            return true;
        }

        char name[MAX_PATH_LENGTH + 1] = {0};
        entry.getName(name, MAX_PATH_LENGTH);
        if (entry.isDirectory()) {
            callback(param, name, "FOLD", entry.size());
        } else if (util::endsWith(name, list::LIST_EXT)) {
            callback(param, name, "LIST", entry.size());
        } else if (util::endsWith(name, profile::PROFILE_EXT)) {
            callback(param, name, "PROF", entry.size());
        } else {
            callback(param, name, "BIN", entry.size());
        }

        entry.close();
    }
}

bool catalogLength(const char *dirPath, size_t *length, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    File dir = SD.open(dirPath);
    if (!dir) {
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    *length = 0;

    dir.rewindDirectory();

    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break;
        }
        ++(*length);
        entry.close();
    }

    return true;
}

bool upload(const char *filePath, void *param, void (*callback)(void *param, const void *buffer, size_t size), int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    File file = SD.open(filePath, FILE_READ);

    if (!file) {
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

	bool result = true;

	size_t totalSize = file.size();

#if OPTION_DISPLAY
	gui::showProgressPage("Uploading...");
	size_t uploaded = 0;
#endif

	*err = SCPI_RES_OK;

    callback(param, NULL, totalSize);

    const int CHUNK_SIZE = CONF_SERIAL_BUFFER_SIZE;
    uint8_t buffer[CHUNK_SIZE];

    while (true) {
        int size = file.read(buffer, CHUNK_SIZE);

		callback(param, buffer, size);

#if OPTION_DISPLAY
		uploaded += size;
		if (!gui::updateProgressPage(uploaded, totalSize)) {
			gui::hideProgressPage();
			event_queue::pushEvent(event_queue::EVENT_WARNING_FILE_UPLOAD_ABORTED);
			if (err) *err = SCPI_ERROR_FILE_TRANSFER_ABORTED;
			result = false;
			break;
		}
#endif

		if (size < CHUNK_SIZE) {
            break;
        }
    }

    file.close();

    callback(param, NULL, -1);

#if OPTION_DISPLAY
	gui::hideProgressPage();
#endif

    return result;
}

bool download(const char *filePath, bool truncate, const void *buffer, size_t size, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    File file = SD.open(filePath, FILE_WRITE);

	if (!file) {
		if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
		return false;
	}

    if (truncate && !file.truncate(0)) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    if (!file) {
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    size_t written = file.write((const uint8_t *)buffer, size);
    file.close();

    if (written != size) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    return true;
}

bool moveFile(const char *sourcePath, const char *destinationPath, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    if (!SD.exists(sourcePath)) {
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    if (!SD.rename(sourcePath, destinationPath)) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    return true;
}

bool copyFile(const char *sourcePath, const char *destinationPath, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    File sourceFile = SD.open(sourcePath, FILE_READ);

    if (!sourceFile) {
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    File destinationFile = SD.open(destinationPath, FILE_WRITE);

    if (!destinationFile) {
        sourceFile.close();
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

#if OPTION_DISPLAY
    gui::showProgressPage("Copying...");
#endif

    const int CHUNK_SIZE = 512;
    uint8_t buffer[CHUNK_SIZE];
    size_t totalSize = sourceFile.size();
    size_t totalWritten = 0;

    while (true) {
        int size = sourceFile.read(buffer, CHUNK_SIZE);

        size_t written = destinationFile.write((const uint8_t *)buffer, size);
        if (size < 0 || written != (size_t)size) {
#if OPTION_DISPLAY
            gui::hideProgressPage();
#endif
            sourceFile.close();
            destinationFile.close();
            deleteFile(destinationPath, NULL);
            if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
            return false;
        }

        totalWritten += written;

#if OPTION_DISPLAY
        if (!gui::updateProgressPage(totalWritten, totalSize)) {
            sourceFile.close();
            destinationFile.close();

            deleteFile(destinationPath, NULL);
            if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;

            gui::hideProgressPage();
            return false;
        }
#endif

        if (size < CHUNK_SIZE) {
            break;
        }

        psu::tick();
    }

    sourceFile.close();
    destinationFile.close();

#if OPTION_DISPLAY
    gui::hideProgressPage();
#endif

    if (totalWritten != totalSize) {
        deleteFile(destinationPath, NULL);
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    return true;
}

bool deleteFile(const char *filePath, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    if (!SD.exists(filePath)) {
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    if (!SD.remove(filePath)) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    return true;
}

bool makeDir(const char *dirPath, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    if (!SD.mkdir(dirPath)) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    return true;
}

bool removeDir(const char *dirPath, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    if (!SD.rmdir(dirPath)) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    return true;
}

void getDateTime(
    dir_t *d,
    uint8_t *resultYear, uint8_t *resultMonth, uint8_t *resultDay,
    uint8_t *resultHour, uint8_t *resultMinute, uint8_t *resultSecond
) {
    int year = FAT_YEAR(d->lastWriteDate);
    int month = FAT_MONTH(d->lastWriteDate);
    int day = FAT_DAY(d->lastWriteDate);

    int hour = FAT_HOUR(d->lastWriteTime);
    int minute = FAT_MINUTE(d->lastWriteTime);
    int second = FAT_SECOND(d->lastWriteTime);

    uint32_t utc = datetime::makeTime(year, month, day, hour, minute, second);
    uint32_t local = datetime::utcToLocal(utc, persist_conf::devConf.time_zone, (datetime::DstRule)persist_conf::devConf2.dstRule);
    datetime::breakTime(local, year, month, day, hour, minute, second);

    if (resultYear) {
        *resultYear = (uint8_t)(year - 2000);
        *resultMonth = (uint8_t)month;
        *resultDay = (uint8_t)day;
    }

    if (resultHour) {
        *resultHour = (uint8_t)hour;
        *resultMinute = (uint8_t)minute;
        *resultSecond = (uint8_t)second;
    }
}

bool getDate(const char *filePath, uint8_t &year, uint8_t &month, uint8_t &day, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    File file = SD.open(filePath, FILE_READ);

    if (!file) {
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    dir_t d;
    bool result = file.dirEntry(&d);
    file.close();

    if (!result) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    getDateTime(&d, &year, &month, &day, NULL, NULL, NULL);

    return true;
}

bool getTime(const char *filePath, uint8_t &hour, uint8_t &minute, uint8_t &second, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    if (!SD.exists(filePath)) {
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    File file = SD.open(filePath, FILE_READ);

    if (!file) {
        if (err) *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    dir_t d;
    bool result = file.dirEntry(&d);
    file.close();

    if (!result) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    getDateTime(&d, NULL, NULL, NULL, &hour, &minute, &second);

    return true;
}

bool getInfo(uint64_t &usedSpace, uint64_t &freeSpace) {
#ifdef EEZ_PSU_SIMULATOR
    return SD.getInfo(usedSpace, freeSpace);
#else
    uint64_t clusterCount = SD.vol()->clusterCount();
    uint64_t freeClusterCount = SD.vol()->freeClusterCount(psu::tick);
    usedSpace = 512 * (clusterCount - freeClusterCount) * SD.vol()->blocksPerCluster();
    freeSpace = 512 * freeClusterCount * SD.vol()->blocksPerCluster();
    return true;
#endif
}

}
}
} // namespace eez::psu::sd_card

#endif
