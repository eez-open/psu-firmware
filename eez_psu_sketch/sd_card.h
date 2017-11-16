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

#include <SD.h>

namespace eez {
namespace psu {
namespace sd_card {

extern TestResult g_testResult;

enum FileType {
    FILE_TYPE_BIN,
    FILE_TYPE_FOLD
};

void init();
bool test();

void matchZeroOrMoreSpaces(File& file);
bool match(File& file, float &result);
bool match(File& file, char c);

bool makeParentDir(const char *filePath);

bool catalog(const char *dirPath, void *param, void (*callback)(void *param, const char *name, FileType type, size_t size), int *err);
bool upload(const char *filePath, void *param, void (*callback)(void *param, const void *buffer, size_t size), int *err);
bool download(const char *filePath, const void *buffer, size_t size, int *err);
bool deleteFile(const char *filePath, int *err);

}
}
} // namespace eez::psu::sd_card
