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

#if defined(EEZ_PLATFORM_SIMULATOR)
#include "eez/app/platform/simulator/SdFat.h"
#endif

#if defined(EEZ_PLATFORM_ARDUINO_DUE)
#include <SdFat.h>
#endif

extern SdFat SD;

namespace eez {
namespace app {
namespace sd_card {

extern TestResult g_testResult;

void init();
bool test();

void dumpInfo(char *buffer);

void matchZeroOrMoreSpaces(File& file);
bool match(File& file, float &result);
bool match(File& file, char c);

bool makeParentDir(const char *filePath);

bool exists(const char *dirPath, int *err);
bool catalog(const char *dirPath, void *param, void (*callback)(void *param, const char *name, const char *type, size_t size), int *err);
bool catalogLength(const char *dirPath, size_t *length, int *err);
bool upload(const char *filePath, void *param, void (*callback)(void *param, const void *buffer, int size), int *err);
bool download(const char *filePath, bool truncate, const void *buffer, size_t size, int *err);
bool moveFile(const char *sourcePath, const char *destinationPath, int *err);
bool copyFile(const char *sourcePath, const char *destinationPath, int *err);
bool deleteFile(const char *filePath, int *err);
bool makeDir(const char *dirPath, int *err);
bool removeDir(const char *dirPath, int *err);
bool getDate(const char *filePath, uint8_t &year, uint8_t &month, uint8_t &day, int *err);
bool getTime(const char *filePath, uint8_t &hour, uint8_t &minute, uint8_t &second, int *err);

bool getInfo(uint64_t &usedSpace, uint64_t &freeSpace);

}
}
} // namespace eez::app::sd_card
