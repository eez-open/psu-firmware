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

#define SCPI_CAL_COMMANDS \
    SCPI_COMMAND("CALibration[:MODE]", scpi_cal_Mode) \
    SCPI_COMMAND("CALibration[:MODE]?", scpi_cal_ModeQ) \
    SCPI_COMMAND("CALibration:CLEar", scpi_cal_Clear) \
    SCPI_COMMAND("CALibration:CURRent[:DATA]", scpi_cal_CurrentData) \
    SCPI_COMMAND("CALibration:CURRent:LEVel", scpi_cal_CurrentLevel) \
    SCPI_COMMAND("CALibration:PASSword:NEW", scpi_cal_PasswordNew) \
    SCPI_COMMAND("CALibration:REMark", scpi_cal_Remark) \
    SCPI_COMMAND("CALibration:REMark?", scpi_cal_RemarkQ) \
    SCPI_COMMAND("CALibration:SAVE", scpi_cal_Save) \
    SCPI_COMMAND("CALibration:STATe", scpi_cal_State) \
    SCPI_COMMAND("CALibration:STATe?", scpi_cal_StateQ) \
    SCPI_COMMAND("CALibration:VOLTage[:DATA]", scpi_cal_VoltageData) \
    SCPI_COMMAND("CALibration:VOLTage:LEVel", scpi_cal_VoltageLevel) \

