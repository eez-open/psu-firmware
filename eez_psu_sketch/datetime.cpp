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
#include "datetime.h"
#include "rtc.h"
#include "scpi_psu.h"

namespace eez {
namespace psu {
namespace datetime {

psu::TestResult test_result = psu::TEST_FAILED;

bool init() {
    return test();
}

int cmp_datetime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second,
    uint8_t rtc_year, uint8_t rtc_month, uint8_t rtc_day, uint8_t rtc_hour, uint8_t rtc_minute, uint8_t rtc_second)
{
    if (year < rtc_year) return -1;
    if (year > rtc_year) return 1;

    if (month < rtc_month) return -1;
    if (month > rtc_month) return 1;

    if (day < rtc_day) return -1;
    if (day > rtc_day) return 1;

    if (hour < rtc_hour) return -1;
    if (hour > rtc_hour) return 1;

    if (minute < rtc_minute) return -1;
    if (minute > rtc_minute) return 1;

    if (second < rtc_second) return -1;
    if (second > rtc_second) return 1;

    return 0;
}

bool test() {
    if (rtc::test_result == psu::TEST_OK) {
        uint8_t year, month, day, hour, minute, second;
        if (persist_conf::readSystemDate(year, month, day) && persist_conf::readSystemTime(hour, minute, second)) {
            uint8_t rtc_year, rtc_month, rtc_day, rtc_hour, rtc_minute, rtc_second;
            rtc::readDateTime(rtc_year, rtc_month, rtc_day, rtc_hour, rtc_minute, rtc_second);
            if (cmp_datetime(year, month, day, hour, minute, second, rtc_year, rtc_month, rtc_day, rtc_hour, rtc_minute, rtc_second) < 0) {
                test_result = psu::TEST_OK;
                persist_conf::writeSystemDate(rtc_year, rtc_month, rtc_day);
                persist_conf::writeSystemTime(rtc_hour, rtc_minute, rtc_second);
            }
            else {
                test_result = psu::TEST_WARNING;
                DebugTraceF("RTC test failed, RTC time (%d-%02d-%02d %02d:%02d:%02d) older then or equal to EEPROM time (%d-%02d-%02d %02d:%02d:%02d)",
                    (int)(2000 + rtc_year), (int)rtc_month, (int)rtc_day, (int)rtc_hour, (int)rtc_minute, (int)rtc_second,
                    (int)(2000 + year), (int)month, (int)day, (int)hour, (int)minute, (int)second);
            }
        }
        else {
            test_result = psu::TEST_SKIPPED;
        }
    }
    else {
        test_result = psu::TEST_SKIPPED;
    }

    psu::setQuesBits(QUES_TIME, test_result != psu::TEST_OK);

    return test_result != psu::TEST_FAILED && test_result != psu::TEST_WARNING;
}

bool isValidDate(uint8_t year, uint8_t month, uint8_t day) {
    if (month < 1 || month > 12) return false;

    if (day < 1 || day > 31) return false;

    if (month == 4 || month == 6 || month == 9 || month == 11) {
        if (day > 30) return false;
    }
    else if (month == 2) {
        bool leap_year = year % 4 == 0;
        if (leap_year) {
            if (day > 29) return false;
        }
        else {
            if (day > 28) return false;
        }
    }

    return true;
}

bool getDate(uint8_t &year, uint8_t &month, uint8_t &day) {
    return rtc::readDate(year, month, day);
}

bool checkDateTime() {
    uint8_t year, month, day, hour, minute, second;
    if (persist_conf::readSystemDate(year, month, day) && persist_conf::readSystemTime(hour, minute, second)) {
        uint8_t rtc_year, rtc_month, rtc_day, rtc_hour, rtc_minute, rtc_second;
        rtc::readDateTime(rtc_year, rtc_month, rtc_day, rtc_hour, rtc_minute, rtc_second);
        return cmp_datetime(year, month, day, hour, minute, second, rtc_year, rtc_month, rtc_day, rtc_hour, rtc_minute, rtc_second) <= 0;
    }
    return false;
}

bool setDate(uint8_t year, uint8_t month, uint8_t day) {
    if (rtc::writeDate(year, month, day)) {
        persist_conf::writeSystemDate(year, month, day);
        psu::setQuesBits(QUES_TIME, !checkDateTime());
        return true;
    }
    return false;
}

bool isValidTime(uint8_t hour, uint8_t minute, uint8_t second) {
    if (hour > 23) return false;
    if (minute > 59) return false;
    if (second > 59) return false;
    return true;
}

bool getTime(uint8_t &hour, uint8_t &minute, uint8_t &second) {
    return rtc::readTime(hour, minute, second);
}

bool setTime(uint8_t hour, uint8_t minute, uint8_t second) {
    if (rtc::writeTime(hour, minute, second)) {
        persist_conf::writeSystemTime(hour, minute, second);
        psu::setQuesBits(QUES_TIME, !checkDateTime());
        return true;
    }
    return false;
}

bool getDateTimeAsString(char *buffer) {
    uint8_t year, month, day, hour, minute, second;
    if (datetime::getDate(year, month, day) && datetime::getTime(hour, minute, second)) {
        sprintf_P(buffer, PSTR("%d-%02d-%02d %02d:%02d:%02d"),
            (int)(year + 2000), (int)month, (int)day,
            (int)hour, (int)minute, (int)second);
        return true;
    }
    return false;
}

}
}
}; // namespace eez::psu::datetime