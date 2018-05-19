/*
* EEZ PSU Firmware
* Copyright (C) 2018-present, Envox d.o.o.
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

/** @file conf_advanced.h
@brief EEZ middleware compile time configuration.
*/

#pragma once

#if !defined(EEZ_PLATFORM_ARDUINO_DUE) && !defined(EEZ_PLATFORM_SIMULATOR) && !defined(EEZ_PLATFORM_STM32)
#define EEZ_PLATFORM_ARDUINO_DUE
#endif

#if defined(EEZ_PLATFORM_ARDUINO_DUE)
#include "eez/psu/platform/arduino_due/psu.h"
#elif defined(EEZ_PLATFORM_SIMULATOR)
#include "eez/psu/platform/simulator/psu.h"
#elif defined(EEZ_PLATFORM_STM32)
#include "eez/psu/platform/stm32/psu.h"
#endif

#include "eez/psu/conf.h"

#if defined(EEZ_PLATFORM_SIMULATOR)
#include "eez/psu/platform/simulator/conf.h"
#elif defined(EEZ_PLATFORM_STM32)
#include "eez/psu/platform/stm32/conf.h"
#endif
