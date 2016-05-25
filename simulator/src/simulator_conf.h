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

#undef SCPI_PARSER_INPUT_BUFFER_LENGTH
#define SCPI_PARSER_INPUT_BUFFER_LENGTH 256

// SIMULATOR SPECIFC CONFIG
#define SIM_LOAD_MIN 0
#define SIM_LOAD_DEF 1000.0f
#define SIM_LOAD_MAX 10000000.0F

#define SIM_TEMP_MIN 0
#define SIM_TEMP_DEF 25.0f
#define SIM_TEMP_MAX 120.0f
