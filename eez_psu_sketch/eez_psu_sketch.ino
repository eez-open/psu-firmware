/**
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
 
#include "SPI.h"

#include <scpi-parser.h>

#include "psu.h"

#if OPTION_ETHERNET
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
#include <UIPEthernet.h>
#include <UIPServer.h>
#include <UIPClient.h>
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
#include <Ethernet2.h>
#include <EthernetServer.h>
#include <EthernetClient.h>
#endif
#endif

#include <UTFT.h>

void setup() {
    PSU_boot();
}

void loop() {
    PSU_tick();
}

