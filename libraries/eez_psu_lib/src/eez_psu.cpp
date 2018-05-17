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

#include <SPI.h>

#include "eez_psu.h"

SPISettings MCP23S08_SPI(4000000, MSBFIRST, SPI_MODE0); // IO EXPANDER
SPISettings DAC8552_SPI(4000000, MSBFIRST, SPI_MODE1); // DAC
SPISettings ADS1120_SPI(4000000, MSBFIRST, SPI_MODE1); // ADC
SPISettings TLC5925_SPI(4000000, MSBFIRST, SPI_MODE0); // Binding Post
SPISettings PCA21125_SPI(4000000, MSBFIRST, SPI_MODE0); // RTC
SPISettings AT25256B_SPI(4000000, MSBFIRST, SPI_MODE0); // EEPROM

#if defined(EEZ_PLATFORM_ARDUINO_DUE)
SPISettings ETHERNET_SPI(F_CPU / 10, MSBFIRST, SPI_MODE0);
#else
SPISettings ETHERNET_SPI(8000000, MSBFIRST, SPI_MODE0);
#endif
