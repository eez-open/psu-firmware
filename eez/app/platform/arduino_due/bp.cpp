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
#include "eez/app/bp.h"

namespace eez {
namespace app {
namespace bp {

extern uint16_t g_lastConf;

void set(uint16_t conf) {
    if (OPTION_BP) {
        SPI_beginTransaction(TLC5925_SPI);
        digitalWrite(BP_OE, HIGH);
        digitalWrite(BP_SELECT, LOW);
        SPI.transfer(conf >> 8);
        SPI.transfer(conf & 0xFF);
        g_lastConf = conf;
        digitalWrite(BP_SELECT, HIGH);
        digitalWrite(BP_SELECT, LOW);
        digitalWrite(BP_OE, LOW);
        SPI_endTransaction();

        //DebugTraceF("BP 0x%04x", (int)conf);
    }
}

}
}
} // namespace eez::app::bp