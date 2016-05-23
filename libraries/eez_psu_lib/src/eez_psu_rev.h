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

#ifndef EEZ_PSU_REV_H
#define EEZ_PSU_REV_H

#define EEZ_PSU_REVISION_R1B9 1
#define EEZ_PSU_REVISION_R2B6 2

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define EEZ_PSU_ARDUINO_MEGA
#elif defined(_VARIANT_ARDUINO_DUE_X_)
#define EEZ_PSU_ARDUINO_DUE
#endif

#endif // EEZ_PSU_REV_H