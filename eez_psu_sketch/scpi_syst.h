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

#define SCPI_SYST_COMMANDS \
    SCPI_COMMAND("SYSTem:CAPability?", scpi_syst_CapabilityQ) \
    SCPI_COMMAND("SYSTem:ERRor[:NEXT]?", scpi_syst_ErrorNextQ) \
    SCPI_COMMAND("SYSTem:ERRor:COUNt?", scpi_syst_ErrorCountQ) \
    SCPI_COMMAND("SYSTem:VERSion?", scpi_syst_VersionQ) \
    SCPI_COMMAND("SYSTem:POWer", scpi_syst_Power) \
    SCPI_COMMAND("SYSTem:POWer?", scpi_syst_PowerQ) \
    SCPI_COMMAND("SYSTem:DATE", scpi_syst_Date) \
    SCPI_COMMAND("SYSTem:DATE?", scpi_syst_DateQ) \
    SCPI_COMMAND("SYSTem:TIME", scpi_syst_Time) \
    SCPI_COMMAND("SYSTem:TIME?", scpi_syst_TimeQ) \
    SCPI_COMMAND("SYSTem:BEEPer[:IMMediate]", scpi_syst_Beeper) \
    SCPI_COMMAND("SYSTem:BEEPer:STATe", scpi_syst_BeeperState) \
    SCPI_COMMAND("SYSTem:BEEPer:STATe?", scpi_syst_BeeperStateQ) \
    SCPI_COMMAND("SYSTem:TEMPerature:PROTection[:HIGH]:CLEar", scpi_syst_TempProtectionClear) \
    SCPI_COMMAND("SYSTem:TEMPerature:PROTection[:HIGH][:LEVel]", scpi_syst_TempProtectionLevel) \
    SCPI_COMMAND("SYSTem:TEMPerature:PROTection[:HIGH][:LEVel]?", scpi_syst_TempProtectionLevelQ) \
    SCPI_COMMAND("SYSTem:TEMPerature:PROTection[:HIGH]:STATe", scpi_syst_TempProtectionState) \
    SCPI_COMMAND("SYSTem:TEMPerature:PROTection[:HIGH]:STATe?", scpi_syst_TempProtectionStateQ  ) \
    SCPI_COMMAND("SYSTem:TEMPerature:PROTection[:HIGH]:DELay[:TIME]", scpi_syst_TempProtectionDelay   ) \
    SCPI_COMMAND("SYSTem:TEMPerature:PROTection[:HIGH]:DELay[:TIME]?", scpi_syst_TempProtectionDelayQ  ) \
    SCPI_COMMAND("SYSTem:TEMPerature:PROTection[:HIGH]:TRIPped?", scpi_syst_TempProtectionTrippedQ) \
    SCPI_COMMAND("SYSTem:CHANnel[:COUNt]?", scpi_syst_ChannelCountQ) \
    SCPI_COMMAND("SYSTem:CHANnel:INFOrmation:CURRent?", scpi_syst_ChannelInformationCurrentQ) \
    SCPI_COMMAND("SYSTem:CHANnel:INFOrmation:POWer?", scpi_syst_ChannelInformationPowerQ) \
    SCPI_COMMAND("SYSTem:CHANnel:INFOrmation:PROGram?", scpi_syst_ChannelInformationProgramQ) \
    SCPI_COMMAND("SYSTem:CHANnel:INFOrmation:VOLTage?", scpi_syst_ChannelInformationVoltageQ) \
    SCPI_COMMAND("SYSTem:CHANnel:MODel?", scpi_syst_ChannelModelQ)
