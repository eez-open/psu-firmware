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

#ifdef OPTION_SD_CARD

#include <SD.h>
#include "sd_card.h"

namespace eez {
namespace psu {
namespace sd_card {

TestResult g_testResult = TEST_FAILED;

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
    File root = SD.open("/");
    printDirectory(root, 0);
}

#endif


}
}
} // namespace eez::psu::sd_card

#endif