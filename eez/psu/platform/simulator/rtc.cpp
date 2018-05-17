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

#include "eez/psu/psu.h"
#include "eez/psu/rtc.h"
#include "eez/psu/datetime.h"

#include <time.h> 

namespace eez {
namespace psu {
namespace rtc {

psu::TestResult g_testResult = psu::TEST_FAILED;
static uint32_t g_offset;

void setOffset(uint32_t offset) {
	g_offset = offset;
	char *file_path = getConfFilePath("RTC.state");
	FILE *fp = fopen(file_path, "wb");
	if (fp != NULL) {
		fseek(fp, 0, SEEK_SET);
		fwrite(&g_offset, sizeof(g_offset), 1, fp);
		fclose(fp);
	}
}

void init() {
	char *file_path = getConfFilePath("RTC.state");
	FILE *fp = fopen(file_path, "r+b");
	if (fp != NULL) {
		fread(&g_offset, sizeof(g_offset), 1, fp);
		fclose(fp);
	} else {
		setOffset(0);
	}
}

bool test() {
	if (OPTION_EXT_RTC) {
		g_testResult = psu::TEST_OK;
	} else {
		g_testResult = psu::TEST_SKIPPED;
	}
	return true;
}

uint32_t nowUtc() {
	time_t now_time_t = time(0);
	struct tm *now_tm = gmtime(&now_time_t);
	return datetime::makeTime(1900 + now_tm->tm_year, now_tm->tm_mon + 1, now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
}

bool readDate(uint8_t &year, uint8_t &month, uint8_t &day) {
	if (g_testResult != psu::TEST_OK) {
		return false;
	}

	int year_, month_, day_, hour_, minute_, second_;
	datetime::breakTime(g_offset + nowUtc(), year_, month_, day_, hour_, minute_, second_);

	year = uint8_t(year_ - 2000);
	month = uint8_t(month_);
	day = uint8_t(day_);

	return true;
}

bool writeDate(uint8_t year, uint8_t month, uint8_t day) {
	if (g_testResult != psu::TEST_OK) {
		return false;
	}

	int year_, month_, day_, hour_, minute_, second_;
	datetime::breakTime(g_offset + nowUtc(), year_, month_, day_, hour_, minute_, second_);
	setOffset(datetime::makeTime(year + 2000, month, day, hour_, minute_, second_) - nowUtc());

	return true;
}

bool readTime(uint8_t &hour, uint8_t &minute, uint8_t &second) {
	if (g_testResult != psu::TEST_OK) {
		return false;
	}

	int year_, month_, day_, hour_, minute_, second_;
	datetime::breakTime(g_offset + nowUtc(), year_, month_, day_, hour_, minute_, second_);

	hour = uint8_t(hour_);
	minute = uint8_t(minute_);
	second = uint8_t(second_);

	return true;
}

bool writeTime(uint8_t hour, uint8_t minute, uint8_t second) {
	if (g_testResult != psu::TEST_OK) {
		return false;
	}

	int year_, month_, day_, hour_, minute_, second_;
	datetime::breakTime(g_offset + nowUtc(), year_, month_, day_, hour_, minute_, second_);
	setOffset(datetime::makeTime(year_, month_, day_, hour, minute, second) - nowUtc());

	return true;
}

bool readDateTime(uint8_t &year, uint8_t &month, uint8_t &day, uint8_t &hour, uint8_t &minute, uint8_t &second) {
	if (g_testResult != psu::TEST_OK) {
		return false;
	}

	int year_, month_, day_, hour_, minute_, second_;
	datetime::breakTime(g_offset + nowUtc(), year_, month_, day_, hour_, minute_, second_);

	year = uint8_t(year_ - 2000);
	month = uint8_t(month_);
	day = uint8_t(day_);

	hour = uint8_t(hour_);
	minute = uint8_t(minute_);
	second = uint8_t(second_);

	return true;
}

bool writeDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
	if (g_testResult != psu::TEST_OK) {
		return false;
	}
	
	setOffset(datetime::makeTime(year + 2000, month, day, hour, minute, second) - nowUtc());

	return true;
}

}
}
} // namespace eez::psu::rtc
