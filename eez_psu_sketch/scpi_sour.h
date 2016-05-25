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

#define SCPI_SOUR_COMMANDS \
    SCPI_COMMAND("[SOURce#]:CURRent[:LEVel][:IMMediate][:AMPLitude]", scpi_source_Current) \
    SCPI_COMMAND("[SOURce#]:CURRent[:LEVel][:IMMediate][:AMPLitude]?", scpi_source_CurrentQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage[:LEVel][:IMMediate][:AMPLitude]", scpi_source_Voltage) \
    SCPI_COMMAND("[SOURce#]:VOLTage[:LEVel][:IMMediate][:AMPLitude]?", scpi_source_VoltageQ) \
    SCPI_COMMAND("[SOURce#]:CURRent[:LEVel][:IMMediate]:STEP[:INCRement]", scpi_source_CurrentStep) \
    SCPI_COMMAND("[SOURce#]:CURRent[:LEVel][:IMMediate]:STEP[:INCRement]?", scpi_source_CurrentStepQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage[:LEVel][:IMMediate]:STEP[:INCRement]", scpi_source_VoltageStep) \
    SCPI_COMMAND("[SOURce#]:VOLTage[:LEVel][:IMMediate]:STEP[:INCRement]?", scpi_source_VoltageStepQ) \
    SCPI_COMMAND("[SOURce#]:CURRent:PROTection:DELay[:TIME]", scpi_source_CurrentProtectionDelay) \
    SCPI_COMMAND("[SOURce#]:CURRent:PROTection:DELay[:TIME]?", scpi_source_CurrentProtectionDelayQ) \
    SCPI_COMMAND("[SOURce#]:CURRent:PROTection:STATe", scpi_source_CurrentProtectionState) \
    SCPI_COMMAND("[SOURce#]:CURRent:PROTection:STATe?", scpi_source_CurrentProtectionStateQ) \
    SCPI_COMMAND("[SOURce#]:CURRent:PROTection:TRIPped?", scpi_source_CurrentProtectionTrippedQ) \
    SCPI_COMMAND("[SOURce#]:POWer:PROTection[:LEVel]", scpi_source_PowerProtectionLevel) \
    SCPI_COMMAND("[SOURce#]:POWer:PROTection[:LEVel]?", scpi_source_PowerProtectionLevelQ) \
    SCPI_COMMAND("[SOURce#]:POWer:PROTection:DELay[:TIME]", scpi_source_PowerProtectionDelay) \
    SCPI_COMMAND("[SOURce#]:POWer:PROTection:DELay[:TIME]?", scpi_source_PowerProtectionDelayQ) \
    SCPI_COMMAND("[SOURce#]:POWer:PROTection:STATe", scpi_source_PowerProtectionState) \
    SCPI_COMMAND("[SOURce#]:POWer:PROTection:STATe?", scpi_source_PowerProtectionStateQ) \
    SCPI_COMMAND("[SOURce#]:POWer:PROTection:TRIPped?", scpi_source_PowerProtectionTrippedQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection:DELay[:TIME]", scpi_source_VoltageProtectionDelay) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection:DELay[:TIME]?", scpi_source_VoltageProtectionDelayQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection:STATe", scpi_source_VoltageProtectionState) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection:STATe?", scpi_source_VoltageProtectionStateQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection:TRIPped?", scpi_source_VoltageProtectionTrippedQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage:SENSe[:SOURce]", scpi_source_VoltageSenseSource) \
    SCPI_COMMAND("[SOURce#]:VOLTage:SENSe[:SOURce]?", scpi_source_VoltageSenseSourceQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROGram[:SOURce]", scpi_source_VoltageProgramSource) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROGram[:SOURce]?", scpi_source_VoltageProgramSourceQ) \

