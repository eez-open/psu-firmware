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

#define SCPI_SIMU_COMMANDS \
    SCPI_COMMAND("SIMUlator:LOAD:STATe", scpi_simu_LoadState) \
    SCPI_COMMAND("SIMUlator:LOAD:STATe?", scpi_simu_LoadStateQ) \
    SCPI_COMMAND("SIMUlator:LOAD", scpi_simu_Load) \
    SCPI_COMMAND("SIMUlator:LOAD?", scpi_simu_LoadQ) \
    SCPI_COMMAND("SIMUlator:VOLTage:PROGram:EXTernal", scpi_simu_VoltageProgramExternal) \
	SCPI_COMMAND("SIMUlator:VOLTage:PROGram:EXTernal?", scpi_simu_VoltageProgramExternalQ) \
    SCPI_COMMAND("SIMUlator:LOAD?", scpi_simu_LoadQ) \
    SCPI_COMMAND("SIMUlator:PWRGood", scpi_simu_Pwrgood) \
    SCPI_COMMAND("SIMUlator:PWRGood?", scpi_simu_PwrgoodQ) \
    SCPI_COMMAND("SIMUlator:TEMPerature", scpi_simu_Temperature) \
    SCPI_COMMAND("SIMUlator:TEMPerature?", scpi_simu_TemperatureQ) \
    SCPI_COMMAND("SIMUlator:GUI", scpi_simu_GUI) \
    SCPI_COMMAND("SIMUlator:EXIT", scpi_simu_Exit) \
    SCPI_COMMAND("SIMUlator:QUIT", scpi_simu_Exit) \

