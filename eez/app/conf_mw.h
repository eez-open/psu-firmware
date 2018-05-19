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

#if !defined(EEZ_PLATFORM_ARDUINO_DUE) && !defined(EEZ_PLATFORM_SIMULATOR) && !defined(EEZ_PLATFORM_STM32)
#define EEZ_PLATFORM_ARDUINO_DUE
#endif

#if defined(EEZ_PLATFORM_ARDUINO_DUE)
#include "eez/app/platform/arduino_due/psu.h"
#elif defined(EEZ_PLATFORM_SIMULATOR)
#include "eez/app/platform/simulator/psu.h"
#elif defined(EEZ_PLATFORM_STM32)
#include "eez/app/platform/stm32/psu.h"
#endif

#include "eez/app/conf_user_revision.h"
#include "eez/app/conf_channel.h"
#include "eez/app/conf.h"
#include "eez/app/conf_advanced.h"
#include "eez/app/conf_user.h"

#if defined(EEZ_PLATFORM_ARDUINO_DUE)
#include "eez_psu.h"
#elif defined(EEZ_PLATFORM_SIMULATOR)
#include "eez/app/platform/simulator/conf.h"
#elif defined(EEZ_PLATFORM_STM32)
#include "eez/app/platform/stm32/conf.h"
#endif
