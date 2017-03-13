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

#if CONF_DEBUG

#define MAX_LEVEL 8

void printDirectory(File dir, int level) {
    // Begin at the start of the directory
    dir.rewindDirectory();
    
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break;
        }
        
        const char *INDENTATION = "    ";
        char indentation[MAX_LEVEL*sizeof(INDENTATION) + 1];
        indentation[0] = 0;
        for (uint8_t i = 0; i < level; ++i) {
            strcat(indentation, INDENTATION);
        }

        if (entry.isDirectory()) {
            DebugTraceF("%s%s/", indentation, entry.name());
            if (level < MAX_LEVEL) {
                printDirectory(entry, level+1);
            }
        } else {
            DebugTraceF("%s%s\t\t%d", indentation, entry.name(), entry.size());
        }

        entry.close();
    }
}

void dir() {
    if (sd_card::g_testResult != TEST_OK) {
        return;
    }

    File root = SD.open("/");
    printDirectory(root, 0);
}

void dumpFile(const char *path) {
    if (sd_card::g_testResult != TEST_OK) {
        return;
    }

    File file = SD.open(path, FILE_READ);

    if (!file) {
        DebugTrace("**ERROR File not found!");
        return;
    }

    char line[256];
    int i = 0;

    while (file.available()) {
        int c = file.read();
        if (c == '\n') {
            line[i] = 0;
            DebugTrace(line);
            i = 0;
        } else {
            if (i < 255) {
                line[i++] = c;
            }
        }
    }
}

#endif


}
}
} // namespace eez::psu::sd_card

#endif