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
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection[:LEVel]", scpi_source_VoltageProtectionLevel) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection[:LEVel]?", scpi_source_VoltageProtectionLevelQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection:DELay[:TIME]", scpi_source_VoltageProtectionDelay) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection:DELay[:TIME]?", scpi_source_VoltageProtectionDelayQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection:STATe", scpi_source_VoltageProtectionState) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection:STATe?", scpi_source_VoltageProtectionStateQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROTection:TRIPped?", scpi_source_VoltageProtectionTrippedQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage:SENSe[:SOURce]", scpi_source_VoltageSenseSource) \
    SCPI_COMMAND("[SOURce#]:VOLTage:SENSe[:SOURce]?", scpi_source_VoltageSenseSourceQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROGram[:SOURce]", scpi_source_VoltageProgramSource) \
    SCPI_COMMAND("[SOURce#]:VOLTage:PROGram[:SOURce]?", scpi_source_VoltageProgramSourceQ) \
    SCPI_COMMAND("[SOURce#]:LRIPple", scpi_source_LRipple) \
    SCPI_COMMAND("[SOURce#]:LRIPple?", scpi_source_LRippleQ) \
    SCPI_COMMAND("[SOURce#]:LRIPple:AUTO", scpi_source_LRippleAuto) \
    SCPI_COMMAND("[SOURce#]:LRIPple:AUTO?", scpi_source_LRippleAutoQ) \
    SCPI_COMMAND("[SOURce#]:CURRent:LIMit[:POSitive][:IMMediate][:AMPLitude]", scpi_source_CurrentLimit) \
    SCPI_COMMAND("[SOURce#]:CURRent:LIMit[:POSitive][:IMMediate][:AMPLitude]?", scpi_source_CurrentLimitQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage:LIMit[:POSitive][:IMMediate][:AMPLitude]", scpi_source_VoltageLimit) \
    SCPI_COMMAND("[SOURce#]:VOLTage:LIMit[:POSitive][:IMMediate][:AMPLitude]?", scpi_source_VoltageLimitQ) \
    SCPI_COMMAND("[SOURce#]:POWer:LIMit", scpi_source_PowerLimit) \
    SCPI_COMMAND("[SOURce#]:POWer:LIMit?", scpi_source_PowerLimitQ) \
    SCPI_COMMAND("[SOURce#]:CURRent[:LEVel]:TRIGgered[:AMPLitude]", scpi_source_CurrentTriggered) \
    SCPI_COMMAND("[SOURce#]:CURRent[:LEVel]:TRIGgered[:AMPLitude]?", scpi_source_CurrentTriggeredQ) \
    SCPI_COMMAND("[SOURce#]:VOLTage[:LEVel]:TRIGgered[:AMPLitude]", scpi_source_VoltageTriggered) \
    SCPI_COMMAND("[SOURce#]:VOLTage[:LEVel]:TRIGgered[:AMPLitude]?", scpi_source_VoltageTriggeredQ) \

