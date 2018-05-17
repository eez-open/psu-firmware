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

void read(uint8_t *buffer, uint16_t buffer_size, uint16_t address) {
    char *file_path = getConfFilePath("EEPROM.state");
    FILE *fp = fopen(file_path, "r+b");
    if (fp == NULL) {
        fp = fopen(file_path, "w+b");
    }    
	if (fp == NULL) {
		return;
	}

    fseek(fp, address, SEEK_SET);
    fread(buffer, 1, buffer_size, fp);
	fclose(fp);
}

bool write(const uint8_t *buffer, uint16_t buffer_size, uint16_t address) {
    char *file_path = getConfFilePath("EEPROM.state");
    FILE *fp = fopen(file_path, "r+b");
    if (fp == NULL) {
        fp = fopen(file_path, "w+b");
    }    
	if (fp == NULL) {
		return false;
	}

    fseek(fp, address, SEEK_SET);
    fwrite(buffer, 1, buffer_size, fp);
    fclose(fp);

	return true;
}

void init() {
}

bool test() {
	if (OPTION_EXT_EEPROM) {
		g_testResult = psu::TEST_OK;
	} else {
		g_testResult = psu::TEST_SKIPPED;
	}
	return true;
}

}
}
} // namespace eez::psu::eeprom
