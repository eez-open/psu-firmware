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

void init();
bool test();

void matchZeroOrMoreSpaces(File& file);
bool match(File& file, float &result);
bool match(File& file, char c);

bool makeParentDir(const char *filePath);

#if CONF_DEBUG
void dir();
void dumpFile(const char *path);
#endif



}
}
} // namespace eez::psu::sd_card
