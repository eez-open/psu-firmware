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

#pragma once

#include <SdFat.h>

extern SdFat SD;

namespace eez {
namespace psu {
namespace sd_card {

extern TestResult g_testResult;

enum FileType {
    FILE_TYPE_BIN,
    FILE_TYPE_TXT,
    FILE_TYPE_STAT,
    FILE_TYPE_FOLD
};

void init();
bool test();

void matchZeroOrMoreSpaces(File& file);
bool match(File& file, float &result);
bool match(File& file, char c);

bool makeParentDir(const char *filePath);

bool exists(const char *dirPath, int *err);
bool catalog(const char *dirPath, void *param, void (*callback)(void *param, const char *name, FileType type, size_t size), int *err);
bool catalogLength(const char *dirPath, size_t *length, int *err);
bool upload(const char *filePath, void *param, void (*callback)(void *param, const void *buffer, size_t size), int *err);
bool download(const char *filePath, const void *buffer, size_t size, int *err);
bool moveFile(const char *sourcePath, const char *destinationPath, int *err);
bool copyFile(const char *sourcePath, const char *destinationPath, int *err);
bool deleteFile(const char *filePath, int *err);
bool makeDir(const char *dirPath, int *err);
bool removeDir(const char *dirPath, int *err);
bool getDate(const char *filePath, uint8_t &year, uint8_t &month, uint8_t &day, int *err);
bool getTime(const char *filePath, uint8_t &hour, uint8_t &minute, uint8_t &second, int *err);

}
}
} // namespace eez::psu::sd_card
