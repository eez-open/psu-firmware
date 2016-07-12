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
 
#include "psu.h"
#include "temp_sensor.h"

namespace eez {
namespace psu {
namespace temp_sensor {

float read(Type sensor) {
#define TEMP_SENSOR(NAME, PIN, CAL_POINTS, CH_NUM, QUES_REG_BIT) \
    if (sensor == NAME) { \
        float value = (float)analogRead(PIN); \
        value = util::remap(value, (float)MIN_ADC, (float)MIN_U, (float)MAX_ADC, (float)MAX_U); \
        return util::remap(value, CAL_POINTS); \
    }

	TEMP_SENSORS

#undef TEMP_SENSOR

    return NAN;
}

}
}
} // namespace eez::psu::temp_sensor