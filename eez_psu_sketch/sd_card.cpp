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

namespace eez {
namespace psu {
namespace sd_card {

TestResult g_testResult = TEST_FAILED;

////////////////////////////////////////////////////////////////////////////////

void init() {
    if (!SD.begin(LCDSD_CS)) {
        g_testResult = TEST_FAILED;
    } else {
        g_testResult = TEST_OK;
    }
}

bool test() {
    return g_testResult != TEST_FAILED;
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

bool catalog(const char *dirPath, void *param, void (*callback)(void *param, const char *name, FileType type, size_t size), int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    File dir = SD.open(dirPath);
    if (!dir) {
        *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    dir.rewindDirectory();

    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            return true;
        }

        if (entry.isDirectory()) {
            callback(param, entry.name(), FILE_TYPE_FOLD, entry.size());
        } else {
            callback(param, entry.name(), FILE_TYPE_BIN, entry.size());
        }

        entry.close();
    }
}

bool upload(const char *filePath, void *param, void (*callback)(void *param, const void *buffer, size_t size), int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    File file = SD.open(filePath, FILE_READ);

    if (!file) {
        *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    callback(param, NULL, file.size());

    const size_t CHUNK_SIZE = 64;
    uint8_t buffer[CHUNK_SIZE];

    while (true) {
        int size = file.read(buffer, CHUNK_SIZE);
        callback(param, buffer, size);
        if (size < CHUNK_SIZE) {
            break;
        }
    }

    file.close();

    callback(param, NULL, 0);

    return true;
}

bool download(const char *filePath, const void *buffer, size_t size, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    File file = SD.open(filePath, FILE_WRITE);

    if (!file) {
        *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    size_t written = file.write((const uint8_t *)buffer, size);
    file.close();

    if (written != size) {
        *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    return true;
}

bool deleteFile(const char *filePath, int *err) {
    if (sd_card::g_testResult != TEST_OK) {
        *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    if (!SD.exists(filePath)) {
        *err = SCPI_ERROR_FILE_NAME_NOT_FOUND;
        return false;
    }

    if (!SD.remove(filePath)) {
        *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    return true;
}

}
}
} // namespace eez::psu::sd_card

#endif