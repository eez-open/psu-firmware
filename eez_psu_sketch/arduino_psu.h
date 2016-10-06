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

#define SCPI_SIMU_COMMANDS

#pragma GCC diagnostic ignored "-Wunused-variable"

#if defined(EEZ_PSU_ARDUINO_DUE)

#ifndef strncmp_P
#define strncmp_P(a, b, c) strncmp((a), (b), (c))
#endif

#ifndef snprintf_P
#define snprintf_P snprintf
#endif

#ifndef vsnprintf_P
#define vsnprintf_P vsnprintf
#endif

#ifndef strncpy_P
#define strncpy_P strncpy
#endif

#endif
