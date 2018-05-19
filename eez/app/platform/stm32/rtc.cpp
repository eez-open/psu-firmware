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
 
#include "eez/app/psu.h"
#include "eez/app/rtc.h"

namespace eez {
namespace app {
namespace rtc {

TestResult g_testResult = TEST_FAILED;

////////////////////////////////////////////////////////////////////////////////

void init() {
}

bool test() {
    if (OPTION_EXT_RTC) {
        g_testResult = TEST_OK;
    }
    else {
        g_testResult = TEST_SKIPPED;
    }

    if (g_testResult == TEST_FAILED) {
        generateError(SCPI_ERROR_RTC_TEST_FAILED);
    }

    return g_testResult != TEST_FAILED;
}

bool readDate(uint8_t &year, uint8_t &month, uint8_t &day) {
    if (g_testResult != TEST_OK) {
        return false;
    }

    day = 1;
    month = 1;
    year = 18;
    
    return true;
}

bool writeDate(uint8_t year, uint8_t month, uint8_t day) {
    if (g_testResult != TEST_OK) {
        return false;
    }

    return true;
}

bool readTime(uint8_t &hour, uint8_t &minute, uint8_t &second) {
    if (g_testResult != TEST_OK) {
        return false;
    }
    
    second = 0;
    minute = 0;
    hour = 10;
    
    return true;
}

bool writeTime(uint8_t hour, uint8_t minute, uint8_t second) {
    if (g_testResult != TEST_OK) {
        return false;
    }
    
    return true;
}

bool readDateTime(uint8_t &year, uint8_t &month, uint8_t &day, uint8_t &hour, uint8_t &minute, uint8_t &second) {
    if (g_testResult != TEST_OK) {
        return false;
    }
    
    second = 0;
    minute = 0;
    hour = 10;
    day = 1;
    month = 1;
    year = 18;
    
    return true;
}

bool writeDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    if (g_testResult != TEST_OK) {
        return false;
    }

    return true;
}

}
}
} // namespace eez::app::rtc
