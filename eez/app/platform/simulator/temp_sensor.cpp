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

#include "eez/app/psu.h"
#include "eez/app/temp_sensor.h"

namespace eez {
namespace app {
namespace temp_sensor {

int temperatureRead(int pin) {
    for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
        temp_sensor::TempSensor &tempSensor = temp_sensor::sensors[i];
        if (tempSensor.installed && tempSensor.pin == pin) {
            float cels = simulator::getTemperature(i);
            int adc = (int)remap(cels, tempSensor.p1_cels, (float)tempSensor.p1_adc, tempSensor.p2_cels, (float)tempSensor.p2_adc);
            return (int)clamp((float)adc, (float)temp_sensor::MIN_ADC, (float)temp_sensor::MAX_ADC);
        }
    }
	return 0;
}

}
}
} // namespace eez::app::temp_sensor