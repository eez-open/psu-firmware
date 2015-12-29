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
 
#pragma once

#define SCPI_MEM_COMMANDS \
    SCPI_COMMAND("MEMory:NSTates?", scpi_mem_NStatesQ) \
    SCPI_COMMAND("MEMory:STATe:CATalog?", scpi_mem_StateCatalogQ) \
    SCPI_COMMAND("MEMory:STATe:DELete", scpi_mem_StateDelete) \
    SCPI_COMMAND("MEMory:STATe:DELete:ALL", scpi_mem_StateDeleteAll) \
    SCPI_COMMAND("MEMory:STATe:NAME", scpi_mem_StateName) \
    SCPI_COMMAND("MEMory:STATe:NAME?", scpi_mem_StateNameQ) \
    SCPI_COMMAND("MEMory:STATe:RECall:AUTO", scpi_mem_StateRecallAuto) \
    SCPI_COMMAND("MEMory:STATe:RECall:AUTO?", scpi_mem_StateRecallAutoQ) \
    SCPI_COMMAND("MEMory:STATe:RECall:SELect", scpi_mem_StateRecallSelect) \
    SCPI_COMMAND("MEMory:STATe:RECall:SELect?", scpi_mem_StateRecallSelectQ) \
    SCPI_COMMAND("MEMory:STATe:VALid?", scpi_mem_StateValidQ) \

