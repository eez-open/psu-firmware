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
namespace rtc {

static const uint8_t RD_CONTROL_1 = 0B10010000;
static const uint8_t WR_CONTROL_1 = 0B00010000;
static const uint8_t RD_CONTROL_2 = 0B10010001;
static const uint8_t WR_CONTROL_2 = 0B00010001;

static const uint8_t RD_SECONDS = 0B10010010;
static const uint8_t WR_SECONDS = 0B00010010;
static const uint8_t RD_MINUTES = 0B10010011;
static const uint8_t WR_MINUTES = 0B00010011;
static const uint8_t RD_HOURS = 0B10010100;
static const uint8_t WR_HOURS = 0B00010100;

static const uint8_t RD_DAYS = 0B10010101;
static const uint8_t WR_DAYS = 0B00010101;
static const uint8_t RD_WEEKDAYS = 0B10010110;
static const uint8_t WR_WEEKDAYS = 0B00010110;
static const uint8_t RD_MONTHS = 0B10010111;
static const uint8_t WR_MONTHS = 0B00010111;
static const uint8_t RD_YEARS = 0B10011000;
static const uint8_t WR_YEARS = 0B00011000;

static const uint8_t RD_CLKOUT = 0B10011101;
static const uint8_t WR_CLKOUT = 0B00011101;

static const uint8_t CONTROL_1_VALUE = 0B00001000;
static const uint8_t CONTROL_2_VALUE = 0B00000000;

extern psu::TestResult test_result;

bool init();
bool test();

bool readDate(uint8_t &year, uint8_t &month, uint8_t &day);
bool writeDate(uint8_t year, uint8_t month, uint8_t day);

bool readTime(uint8_t &hour, uint8_t &minute, uint8_t &second);
bool writeTime(uint8_t hour, uint8_t minute, uint8_t second);

bool readDateTime(uint8_t &year, uint8_t &month, uint8_t &day, uint8_t &hour, uint8_t &minute, uint8_t &second);

}
}
} // namespace eez::psu::rtc
