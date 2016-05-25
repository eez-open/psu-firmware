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

#define SCPI_DIAG_COMMANDS \
    SCPI_COMMAND("DIAGnostic[:INFOrmation]:ADC?",         scpi_diag_InformationADCQ) \
    SCPI_COMMAND("DIAGnostic[:INFOrmation]:CALibration?", scpi_diag_InformationCalibrationQ) \
    SCPI_COMMAND("DIAGnostic[:INFOrmation]:PROTection?",  scpi_diag_InformationProtectionQ) \
    SCPI_COMMAND("DIAGnostic[:INFOrmation]:TEST?",        scpi_diag_InformationTestQ) \

