/*
 * EEZ PSU Firmware
 * Copyright (C) 2017-present, Envox d.o.o.
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

#define SCPI_TRIGGER_COMMANDS \
    SCPI_COMMAND("TRIGger[:SEQuence][:IMMediate]", scpi_trigger_SequenceImmediate) \
    SCPI_COMMAND("TRIGger[:SEQuence]:DELay", scpi_trigger_SequenceDelay) \
    SCPI_COMMAND("TRIGger[:SEQuence]:DELay?", scpi_trigger_SequenceDelayQ) \
    SCPI_COMMAND("TRIGger[:SEQuence]:SLOPe", scpi_trigger_SequenceSlope) \
    SCPI_COMMAND("TRIGger[:SEQuence]:SLOPe?", scpi_trigger_SequenceSlopeQ) \
    SCPI_COMMAND("TRIGger[:SEQuence]:SOURce", scpi_trigger_SequenceSource) \
    SCPI_COMMAND("TRIGger[:SEQuence]:SOURce?", scpi_trigger_SequenceSourceQ) \
    SCPI_COMMAND("INITiate", scpi_trigger_Initiate) \
    SCPI_COMMAND("INITiate:CONTinuous", scpi_trigger_InitiateContinuous) \
    SCPI_COMMAND("INITiate:CONTinuous?", scpi_trigger_InitiateContinuousQ) \
    SCPI_COMMAND("ABORT", scpi_trigger_Abort) \
    SCPI_COMMAND("*TRG", scpi_trigger_Trg)
