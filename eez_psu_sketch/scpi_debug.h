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

#if CONF_DEBUG

#define SCPI_DEBUG_COMMANDS \
    SCPI_COMMAND("DEBUG", debug_scpi_command) \
    SCPI_COMMAND("DEBUG?", debug_scpi_commandQ) \
	SCPI_COMMAND("DEBUG:WDOG", debug_scpi_Watchdog) \
	SCPI_COMMAND("DEBUG:WDOG?", debug_scpi_WatchdogQ) \
	SCPI_COMMAND("DEBUG:ONTime?", debug_scpi_OntimeQ) \

#else // NO DEBUG

#define SCPI_DEBUG_COMMANDS

#endif
