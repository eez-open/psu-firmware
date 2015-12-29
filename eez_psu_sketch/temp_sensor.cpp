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
 
#include "psu.h"
#include "temp_sensor.h"

namespace eez {
namespace psu {
namespace temp_sensor {

float read(Type sensor) {
    if (sensor == MAIN) {
        float value = (float)analogRead(TEMP_ANALOG);

        // convert to voltage
        value = util::remap(value, (float)MIN_ADC, (float)MIN_U, (float)MAX_ADC, (float)MAX_U);

        // MAIN_TEMP_COEF_P1, MAIN_TEMP_COEF_P2 are defined in conf.h
        return util::remap(value, MAIN_TEMP_COEF_P1_U, MAIN_TEMP_COEF_P1_T, MAIN_TEMP_COEF_P2_U, MAIN_TEMP_COEF_P2_T);
    }
    else {
        // return NaN
        return NAN;
    }
}

}
}
} // namespace eez::psu::temp_sensor