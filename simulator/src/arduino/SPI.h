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

#include "Arduino.h"

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

namespace eez {
namespace psu {
namespace simulator {
namespace arduino {

/// Bare minimum implementation of the Arduino SPISettings class
class SPISettings {
public:
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode);
};

/// Bare minimum implementation of the Arduino SPI object
class SimulatorSPI {
public:
    void begin();

    void usingInterrupt(uint8_t interruptNumber);
    void beginTransaction(SPISettings settings);
    uint8_t transfer(uint8_t data);
    void endTransaction(void);
    void attachInterrupt();

	void setBitOrder(int _order);
	void setDataMode(uint8_t _mode);
	void setClockDivider(uint8_t _div);
};

extern SimulatorSPI SPI;

}
}
}
} // namespace eez::psu::simulator::arduino;
