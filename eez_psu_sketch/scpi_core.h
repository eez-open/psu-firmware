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

#define SCPI_CORE_COMMANDS \
    SCPI_COMMAND("*CLS",  scpi_core_Cls) \
    SCPI_COMMAND("*ESE",  scpi_core_Ese) \
    SCPI_COMMAND("*ESE?", scpi_core_EseQ) \
    SCPI_COMMAND("*ESR?", scpi_core_EsrQ) \
    SCPI_COMMAND("*IDN?", scpi_core_IdnQ) \
    SCPI_COMMAND("*OPC",  scpi_core_Opc) \
    SCPI_COMMAND("*OPC?", scpi_core_OpcQ) \
    SCPI_COMMAND("*RCL",  scpi_core_Rcl) \
    SCPI_COMMAND("*RST",  scpi_core_Rst) \
    SCPI_COMMAND("*SAV",  scpi_core_Sav) \
    SCPI_COMMAND("*SRE",  scpi_core_Sre) \
    SCPI_COMMAND("*SRE?", scpi_core_SreQ) \
    SCPI_COMMAND("*STB?", scpi_core_StbQ) \
    SCPI_COMMAND("*TST?", scpi_core_TstQ) \
    SCPI_COMMAND("*WAI",  scpi_core_Wai) \

