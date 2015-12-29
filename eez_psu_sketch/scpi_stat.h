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

#define SCPI_STAT_COMMANDS \
    SCPI_COMMAND("STATus:QUEStionable[:EVENt]?", scpi_stat_QuestionableEventQ) \
    SCPI_COMMAND("STATus:QUEStionable:CONDition?", scpi_stat_QuestionableConditionQ) \
    SCPI_COMMAND("STATus:QUEStionable:ENABle", scpi_stat_QuestionableEnable) \
    SCPI_COMMAND("STATus:QUEStionable:ENABle?", scpi_stat_QuestionableEnableQ) \
    SCPI_COMMAND("STATus:OPERation[:EVENt]?", scpi_stat_OperationEventQ) \
    SCPI_COMMAND("STATus:OPERation:CONDition?", scpi_stat_OperationConditionQ) \
    SCPI_COMMAND("STATus:OPERation:ENABle", scpi_stat_OperationEnable) \
    SCPI_COMMAND("STATus:OPERation:ENABle?", scpi_stat_OperationEnableQ) \
    SCPI_COMMAND("STATus:QUEStionable:INSTrument[:EVENt]?", scpi_stat_QuestionableInstrumentEventQ) \
    SCPI_COMMAND("STATus:QUEStionable:INSTrument:CONDition?", scpi_stat_QuestionableInstrumentConditionQ) \
    SCPI_COMMAND("STATus:QUEStionable:INSTrument:ENABle", scpi_stat_QuestionableInstrumentEnable) \
    SCPI_COMMAND("STATus:QUEStionable:INSTrument:ENABle?", scpi_stat_QuestionableInstrumentEnableQ) \
    SCPI_COMMAND("STATus:OPERation:INSTrument[:EVENt]?", scpi_stat_OperationInstrumentEventQ) \
    SCPI_COMMAND("STATus:OPERation:INSTrument:CONDition?", scpi_stat_OperationInstrumentConditionQ) \
    SCPI_COMMAND("STATus:OPERation:INSTrument:ENABle", scpi_stat_OperationInstrumentEnable) \
    SCPI_COMMAND("STATus:OPERation:INSTrument:ENABle?", scpi_stat_OperationInstrumentEnableQ) \
    SCPI_COMMAND("STATus:QUEStionable:INSTrument:ISUMmary#[:EVENt]?", scpi_stat_QuestionableInstrumentISummaryEventQ) \
    SCPI_COMMAND("STATus:QUEStionable:INSTrument:ISUMmary#:CONDition?", scpi_stat_QuestionableInstrumentISummaryConditionQ) \
    SCPI_COMMAND("STATus:QUEStionable:INSTrument:ISUMmary#:ENABle", scpi_stat_QuestionableInstrumentISummaryEnable) \
    SCPI_COMMAND("STATus:QUEStionable:INSTrument:ISUMmary#:ENABle?", scpi_stat_QuestionableInstrumentISummaryEnableQ) \
    SCPI_COMMAND("STATus:OPERation:INSTrument:ISUMmary#[:EVENt]?", scpi_stat_OperationInstrumentISummaryEventQ) \
    SCPI_COMMAND("STATus:OPERation:INSTrument:ISUMmary#:CONDition?", scpi_stat_OperationInstrumentISummaryConditionQ) \
    SCPI_COMMAND("STATus:OPERation:INSTrument:ISUMmary#:ENABle", scpi_stat_OperationInstrumentISummaryEnable) \
    SCPI_COMMAND("STATus:OPERation:INSTrument:ISUMmary#:ENABle?", scpi_stat_OperationInstrumentISummaryEnableQ) \
    SCPI_COMMAND("STATus:PREset", scpi_stat_Preset) \

