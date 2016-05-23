/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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

SPISettings MCP23S08_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);
SPISettings DAC8552_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE1);
SPISettings ADS1120_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE1);
SPISettings TLC5925_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);
SPISettings PCA21125_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);
SPISettings AT25256B_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);
#if defined(EEZ_PSU_ARDUINO_DUE)
SPISettings ENC28J60_SPI(10, MSBFIRST, SPI_MODE0);
#else
SPISettings ENC28J60_SPI(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0);
#endif
