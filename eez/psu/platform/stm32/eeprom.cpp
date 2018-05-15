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
#include "eez/psu/eeprom.h"

namespace eez {
namespace psu {
namespace eeprom {

psu::TestResult g_testResult = psu::TEST_FAILED;

////////////////////////////////////////////////////////////////////////////////

static const uint16_t EEPROM_TEST_ADDRESS = 0;
static const uint16_t EEPROM_TEST_BUFFER_SIZE = 64;

////////////////////////////////////////////////////////////////////////////////

void read(uint8_t *buffer, uint16_t buffer_size, uint16_t address) {
}

bool write(const uint8_t *buffer, uint16_t buffer_size, uint16_t address) {
	return true;
}

void init() {
}

bool test() {
    if (OPTION_EXT_EEPROM) {
        g_testResult = psu::TEST_OK;
    }
    else {
        g_testResult = psu::TEST_SKIPPED;
    }

    if (g_testResult == psu::TEST_FAILED) {
        psu::generateError(SCPI_ERROR_EXT_EEPROM_TEST_FAILED);
    }

    return g_testResult != psu::TEST_FAILED;
}

}
}
} // namespace eez::psu::eeprom
