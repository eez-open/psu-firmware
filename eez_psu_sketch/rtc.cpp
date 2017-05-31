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
#include "rtc.h"

namespace eez {
namespace psu {
namespace rtc {

psu::TestResult g_testResult = psu::TEST_FAILED;

////////////////////////////////////////////////////////////////////////////////

void readRegisters(int command, int n, uint8_t *values) {
    SPI_beginTransaction(PCA21125_SPI);
    digitalWrite(RTC_SELECT, HIGH); // Select PCA21125
    SPI.transfer(command); // Read mode, a pointer to the address 02h
    while (n--) {
        *values++ = SPI.transfer(0x00);
    }
    digitalWrite(RTC_SELECT, LOW); // Deselect PCA21125
    SPI_endTransaction();
}

void writeRegisters(int command, int n, const uint8_t *values) {
    SPI_beginTransaction(PCA21125_SPI);
    digitalWrite(RTC_SELECT, HIGH); // Select PCA21125
    SPI.transfer(command); // Read mode, a pointer to the address 02h
    while (n--) {
        SPI.transfer(*values++);
    }
    digitalWrite(RTC_SELECT, LOW); // Deselect PCA21125
    SPI_endTransaction();
}

void init() {
}

bool test() {
    if (OPTION_EXT_RTC) {
        byte ctrl_reg_values[2] = {
            CONTROL_1_VALUE,
            CONTROL_2_VALUE
        };

        writeRegisters(WR_CONTROL_1, 2, ctrl_reg_values);
        readRegisters(RD_CONTROL_1, 2, ctrl_reg_values);

        g_testResult = psu::TEST_OK;

        if (ctrl_reg_values[0] != CONTROL_1_VALUE) {
            DebugTraceF("RTC test failed Control 1: w=%d, r=%d", (int)CONTROL_1_VALUE, (int)ctrl_reg_values[0]);
            g_testResult = psu::TEST_FAILED;
        }

        if (ctrl_reg_values[1] != CONTROL_2_VALUE) {
            DebugTraceF("RTC test failed Control 2: w=%d, r=%d", (int)CONTROL_2_VALUE, (int)ctrl_reg_values[1]);
            g_testResult = psu::TEST_FAILED;
        }
    }
    else {
        g_testResult = psu::TEST_SKIPPED;
    }

    if (g_testResult == psu::TEST_FAILED) {
        psu::generateError(SCPI_ERROR_RTC_TEST_FAILED);
    }

    return g_testResult != psu::TEST_FAILED;
}

bool readDate(uint8_t &year, uint8_t &month, uint8_t &day) {
    if (g_testResult != psu::TEST_OK) {
        return false;
    }

    uint8_t data[4];
    readRegisters(RD_DAYS, sizeof(data), data);
    
    day = util::fromBCD(data[0]);
    month = util::fromBCD(data[2]);
    year = util::fromBCD(data[3]);
    
    return true;
}

bool writeDate(uint8_t year, uint8_t month, uint8_t day) {
    if (g_testResult != psu::TEST_OK) {
        return false;
    }

    uint8_t data[4] = {
        util::toBCD(day),
        0,
        util::toBCD(month),
        util::toBCD(year),
    };

    writeRegisters(WR_DAYS, sizeof(data), data);

    return true;
}

bool readTime(uint8_t &hour, uint8_t &minute, uint8_t &second) {
    if (g_testResult != psu::TEST_OK) {
        return false;
    }
    
    uint8_t data[3];
    readRegisters(RD_SECONDS, sizeof(data), data);
    
    second = util::fromBCD(data[0] & 0x7F);
    minute = util::fromBCD(data[1]);
    hour = util::fromBCD(data[2]);
    
    return true;
}

bool writeTime(uint8_t hour, uint8_t minute, uint8_t second) {
    if (g_testResult != psu::TEST_OK) {
        return false;
    }

    uint8_t data[3] = {
        util::toBCD(second),
        util::toBCD(minute),
        util::toBCD(hour)
    };
    
    writeRegisters(WR_SECONDS, sizeof(data), data);
    
    return true;
}

bool readDateTime(uint8_t &year, uint8_t &month, uint8_t &day, uint8_t &hour, uint8_t &minute, uint8_t &second) {
    if (g_testResult != psu::TEST_OK) {
        return false;
    }
    
    uint8_t data[7];
    readRegisters(RD_SECONDS, sizeof(data), data);
    
    second = util::fromBCD(data[0] & 0x7F);
    minute = util::fromBCD(data[1]);
    hour = util::fromBCD(data[2]);
    day = util::fromBCD(data[3]);
    month = util::fromBCD(data[5]);
    year = util::fromBCD(data[6]);
    
    return true;
}

bool writeDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    if (g_testResult != psu::TEST_OK) {
        return false;
    }

    uint8_t data[7] = {
        util::toBCD(second),
        util::toBCD(minute),
        util::toBCD(hour),
        util::toBCD(day),
        0,
        util::toBCD(month),
        util::toBCD(year),
    };

    writeRegisters(WR_SECONDS, sizeof(data), data);

    return true;
}

}
}
} // namespace eez::psu::rtc