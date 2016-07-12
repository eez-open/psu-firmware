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

#if OPTION_MAIN_TEMP_SENSOR
#define MAIN_SENSOR TEMP_SENSOR(MAIN, TEMP_ANALOG, MAIN_TEMP_SENSOR_CALIBRATION_POINTS, -1, QUES_TEMP)
#else
#define MAIN_SENSOR
#endif

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9

#define TEMP_SENSORS \
	MAIN_SENSOR

#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6

#define TEMP_SENSORS \
	MAIN_SENSOR \
	TEMP_SENSOR(CH1, NTC1, CH1_TEMP_SENSOR_CALIBRATION_POINTS, 0, QUES_ISUM_TEMP) \
	TEMP_SENSOR(CH2, NTC2, CH2_TEMP_SENSOR_CALIBRATION_POINTS, 1, QUES_ISUM_TEMP)

#endif

namespace eez {
namespace psu {
namespace temp_sensor {

static const int MIN_ADC = 0;
static const int MIN_U = 0;
static const int MAX_ADC = 1023;
static const int MAX_U = 5;

static const int MAX_NUM_TEMP_SENSORS = 5;

#define TEMP_SENSOR(NAME, PIN, CAL_POINTS, CH_NUM, QUES_REG_BIT) NAME,

enum Type {
TEMP_SENSORS
TEMP_SENSORS_COUNT
};

#undef TEMP_SENSOR

float read(Type sensor);

}
}
} // namespace eez::psu::temp_sensor
