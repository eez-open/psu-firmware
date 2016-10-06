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
#include "event_queue.h"
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

bool isValidTime(uint8_t hour, uint8_t minute, uint8_t second) {
    if (hour > 23) return false;
    if (minute > 59) return false;
    if (second > 59) return false;
    return true;
}

bool test() {
    if (rtc::test_result == psu::TEST_OK) {
        uint8_t year, month, day, hour, minute, second;
        if (persist_conf::readSystemDate(year, month, day) && persist_conf::readSystemTime(hour, minute, second)) {
            uint8_t rtc_year, rtc_month, rtc_day, rtc_hour, rtc_minute, rtc_second;
            rtc::readDateTime(rtc_year, rtc_month, rtc_day, rtc_hour, rtc_minute, rtc_second);

			if (!isValidDate(2000 + rtc_year, rtc_month, rtc_day)) {
                test_result = psu::TEST_FAILED;
                DebugTraceF("RTC test failed, invalid date format detected (%d-%02d-%02d)",
                    (int)(2000 + rtc_year), (int)rtc_month, (int)rtc_day);
			} else if (!isValidTime(rtc_hour, rtc_minute, rtc_second)) {
                test_result = psu::TEST_FAILED;
                DebugTraceF("RTC test failed, invalid time format detected (%02d:%02d:%02d)",
                    (int)rtc_hour, (int)rtc_minute, (int)rtc_second);
			} else if (cmp_datetime(year, month, day, hour, minute, second, rtc_year, rtc_month, rtc_day, rtc_hour, rtc_minute, rtc_second) < 0) {
                test_result = psu::TEST_OK;
                persist_conf::writeSystemDate(rtc_year, rtc_month, rtc_day);
                persist_conf::writeSystemTime(rtc_hour, rtc_minute, rtc_second);
            }
            else {
                test_result = psu::TEST_FAILED;
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
		event_queue::pushEvent(event_queue::EVENT_INFO_SYSTEM_DATE_TIME_CHANGED);
        return true;
    }
    return false;
}

bool getTime(uint8_t &hour, uint8_t &minute, uint8_t &second) {
    return rtc::readTime(hour, minute, second);
}

bool setTime(uint8_t hour, uint8_t minute, uint8_t second) {
    if (rtc::writeTime(hour, minute, second)) {
        persist_conf::writeSystemTime(hour, minute, second);
        psu::setQuesBits(QUES_TIME, !checkDateTime());
		event_queue::pushEvent(event_queue::EVENT_INFO_SYSTEM_DATE_TIME_CHANGED);
        return true;
    }
    return false;
}

bool getDateTime(uint8_t &year, uint8_t &month, uint8_t &day, uint8_t &hour, uint8_t &minute, uint8_t &second) {
	return rtc::readDateTime(year, month, day, hour, minute, second);
}

bool setDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    if (rtc::writeDateTime(year, month, day, hour, minute, second)) {
		persist_conf::writeSystemDateTime(year, month, day, hour, minute, second);
        psu::setQuesBits(QUES_TIME, !checkDateTime());
		event_queue::pushEvent(event_queue::EVENT_INFO_SYSTEM_DATE_TIME_CHANGED);
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

#define SECONDS_PER_MINUTE 60UL
#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * 60)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * 24)

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y) ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

// API starts months from 1, this array starts from 0
static const uint8_t monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
 
uint32_t now() {
	uint8_t year, month, day, hour, minute, second;
	rtc::readDateTime(year, month, day, hour, minute, second);
	return makeTime(2000 + year, month, day, hour, minute, second);
}

uint32_t makeTime(int year, int month, int day, int hour, int minute, int second) {
	// seconds from 1970 till 1 jan 00:00:00 of the given year
	year -= 1970;

	uint32_t seconds = year * 365 * SECONDS_PER_DAY;

	for (int i = 0; i < year; i++) {
		if (LEAP_YEAR(i)) {
			seconds += SECONDS_PER_DAY; // add extra days for leap years
		}
	}
  
	// add days for this year, months start from 1
	for (int i = 1; i < month; i++) {
		if ((i == 2) && LEAP_YEAR(year)) { 
			seconds += SECONDS_PER_DAY * 29;
		} else {
			seconds += SECONDS_PER_DAY * monthDays[i - 1];  // monthDay array starts from 0
		}
	}
	seconds += (day-1) * SECONDS_PER_DAY;
	seconds += hour * SECONDS_PER_HOUR;
	seconds += minute * SECONDS_PER_MINUTE;
	seconds += second;
	
	return seconds; 
}

void breakTime(uint32_t time, int &resultYear, int &resultMonth, int &resultDay, int &resultHour, int &resultMinute, int &resultSecond) {
	// break the given time_t into time components
	uint8_t year;
	uint8_t month, monthLength;
	unsigned long days;

	resultSecond = time % 60;
	time /= 60; // now it is minutes

	resultMinute = time % 60;
	time /= 60; // now it is hours

	resultHour = time % 24;
	time /= 24; // now it is days
  
	year = 0;  
	days = 0;
	while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
		year++;
	}
	resultYear = year + 1970; // year is offset from 1970 
  
	days -= LEAP_YEAR(year) ? 366 : 365;
	time -= days; // now it is days in this year, starting at 0
  
	days = 0;
	month = 0;
	monthLength = 0;
	for (month = 0; month < 12; ++month) {
		if (month == 1) { // february
			if (LEAP_YEAR(year)) {
				monthLength = 29;
			} else {
				monthLength = 28;
			}
		} else {
			monthLength = monthDays[month];
		}
    
		if (time >= monthLength) {
			time -= monthLength;
		} else {
			break;
		}
	}

	resultMonth = month + 1;  // jan is month 1  
	resultDay = time + 1;     // day of month
}

////////////////////////////////////////////////////////////////////////////////

DateTime::DateTime() {
}

DateTime::DateTime(uint16_t year_, uint8_t month_, uint8_t day_, uint8_t hour_, uint8_t minute_, uint8_t second_)
	: year(year_), month(month_), day(day_), hour(hour_), minute(minute_), second(second_)
{
}

DateTime::DateTime(const DateTime& rhs) {
	memcpy(this, &rhs, sizeof(DateTime));
}

DateTime DateTime::now() {
	uint8_t year, month, day, hour, minute, second;

	if (!getDateTime(year, month, day, hour, minute, second)) {
		year = 16;
		month = 10;
		day = 1;
		hour = 0;
		minute = 0;
		second = 0;
	}

	return DateTime(2000 + year, month, day, hour, minute, second);
}

bool DateTime::operator ==(const DateTime &rhs) {
	return memcmp(this, &rhs, sizeof(DateTime)) == 0;
}

bool DateTime::operator !=(const DateTime &rhs) {
	return memcmp(this, &rhs, sizeof(DateTime)) != 0;
}

}
}
}; // namespace eez::psu::datetime