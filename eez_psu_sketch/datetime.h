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
 
#pragma once

namespace eez {
namespace psu {
/// System date and time.
namespace datetime {

extern psu::TestResult g_testResult;

void init();
bool test();

bool isValidDate(uint8_t year, uint8_t month, uint8_t day);
bool getDate(uint8_t &year, uint8_t &month, uint8_t &day);
bool setDate(uint8_t year, uint8_t month, uint8_t day);

bool isValidTime(uint8_t hour, uint8_t minute, uint8_t second);
bool getTime(uint8_t &hour, uint8_t &minute, uint8_t &second);
bool setTime(uint8_t hour, uint8_t minute, uint8_t second);

bool getDateTime(uint8_t &year, uint8_t &month, uint8_t &day, uint8_t &hour, uint8_t &minute, uint8_t &second);
bool setDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

/// Returns date time as string in format YYYY-MM-DD HH:MM:SS.
/// \param buffer Pointer to the buffer of at least 20 characters. 
/// \returns true if successful.
bool getDateTimeAsString(char *buffer);

uint32_t now();
uint32_t makeTime(int year, int month, int day, int hour, int minute, int second);
void breakTime(uint32_t time, int &resultYear, int &resultMonth, int &resultDay, int &resultHour, int &resultMinute, int &resultSecond);

struct DateTime {
    uint16_t year;
    uint8_t month, day, hour, minute, second;

    DateTime();
    DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    DateTime(const DateTime& rhs);

    static DateTime now();

    bool operator ==(const DateTime &rhs);
    bool operator !=(const DateTime &rhs);
};

}
}
}; // namespace eez::psu::datetime