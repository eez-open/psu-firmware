/*
 * EEZ PSU Firmware
 * Copyright (C) 2016-present, Envox d.o.o.
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
 
/** @file conf_user.h
@brief EEZ PSU revision specification.
Use this header file to specify EEZ PSU revision.
*/

#pragma once

/// Selected EEZ PSU revison, possible values are:
///   - EEZ_PSU_REVISION_R1B9
///   - EEZ_PSU_REVISION_R3B4 (default)
#define EEZ_PSU_SELECTED_REVISION EEZ_PSU_REVISION_R3B4
