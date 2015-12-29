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

namespace eez {
namespace psu {
namespace temp_sensor {

static const int MIN_ADC = 0;
static const int MIN_U = 0;
static const int MAX_ADC = 1023;
static const int MAX_U = 5;

enum Type {
    MAIN,
    S1,
    S2,
    BAT1,
    BAT2,
    COUNT
};

float read(Type sensor);

}
}
} // namespace eez::psu::temp_sensor
