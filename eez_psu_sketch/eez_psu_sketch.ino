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
#include <UIPEthernet.h>
#include <UIPServer.h>
#include <UIPClient.h>
#include <scpi-parser.h>
#include <eez_psu_rev.h>
#include <eez_psu.h>
#include "UTFT.h"

void PSU_boot();
void PSU_tick();

void setup() {
    PSU_boot();
}

void loop() {
    PSU_tick();
}

