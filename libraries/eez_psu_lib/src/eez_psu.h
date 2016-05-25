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

#ifndef EEZ_PSU_H
#define EEZ_PSU_H

#include "eez_psu_rev.h"

#if !defined(EEZ_PSU_SELECTED_REVISION)
#define EEZ_PSU_SELECTED_REVISION EEZ_PSU_REVISION_R1B9
#endif

////////////////////////////////////////////////////////////////////////////////

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    #include "R1B9/R1B9_pins.h"
    
    extern void eez_psu_R1B9_init();
    #define eez_psu_init eez_psu_R1B9_init
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
    #include "R2B6/R2B6_pins.h"
    
    extern void eez_psu_R2B6_init();
    #define eez_psu_init eez_psu_R2B6_init
#else
    #error "Unknown EEZ PSU Revision"
#endif

////////////////////////////////////////////////////////////////////////////////
// IO EXPANDER - MCP23S08

extern SPISettings MCP23S08_SPI;

////////////////////////////////////////////////////////////////////////////////
// DAC - DAC8552

extern SPISettings DAC8552_SPI;

////////////////////////////////////////////////////////////////////////////////
// ADC - ADS1120

extern SPISettings ADS1120_SPI;

////////////////////////////////////////////////////////////////////////////////
// BP - TLC5925

extern SPISettings TLC5925_SPI;

////////////////////////////////////////////////////////////////////////////////
// RTC - PCA21125

extern SPISettings PCA21125_SPI;

////////////////////////////////////////////////////////////////////////////////
// EEPROM - AT25256B

extern SPISettings AT25256B_SPI;

////////////////////////////////////////////////////////////////////////////////
// ETHERNET - ENC28J60

extern SPISettings ENC28J60_SPI;

////////////////////////////////////////////////////////////////////////////////

#endif // EEZ_PSU_H