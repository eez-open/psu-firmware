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

/// temperature sensors
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9

#define TEMP_SENSORS \
    TEMP_SENSOR(AUX, OPTION_AUX_TEMP_SENSOR, TEMP_ANALOG, AUX_TEMP_SENSOR_CALIBRATION_POINTS, -1, QUES_TEMP, SCPI_ERROR_AUX_TEMP_SENSOR_TEST_FAILED)

#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12

#define TEMP_SENSORS \
    TEMP_SENSOR(AUX, OPTION_AUX_TEMP_SENSOR, TEMP_ANALOG, AUX_TEMP_SENSOR_CALIBRATION_POINTS, -1, QUES_TEMP, SCPI_ERROR_AUX_TEMP_SENSOR_TEST_FAILED), \
    TEMP_SENSOR(CH1, CH_NUM >= 1, NTC1, CH1_TEMP_SENSOR_CALIBRATION_POINTS, 0, QUES_ISUM_TEMP, SCPI_ERROR_CH1_TEMP_SENSOR_TEST_FAILED), \
    TEMP_SENSOR(CH2, CH_NUM >= 2, NTC2, CH2_TEMP_SENSOR_CALIBRATION_POINTS, 1, QUES_ISUM_TEMP, SCPI_ERROR_CH2_TEMP_SENSOR_TEST_FAILED)

#endif

namespace eez {
namespace app {
namespace temp_sensor {

static const int MIN_ADC = 0;
static const int MIN_U = 0;
static const int MAX_ADC = 1023;
static const int MAX_U = 5;

////////////////////////////////////////////////////////////////////////////////

static const int MAX_NUM_TEMP_SENSORS = 5;

////////////////////////////////////////////////////////////////////////////////

#define TEMP_SENSOR(NAME, INSTALLED, PIN, CAL_POINTS, CH_NUM, QUES_REG_BIT, SCPI_ERROR) NAME
enum Type {
	TEMP_SENSORS,
	NUM_TEMP_SENSORS
};
#undef TEMP_SENSOR

////////////////////////////////////////////////////////////////////////////////

class TempSensor {
public:
	TempSensor(uint8_t index_, const char *name, int installed, int pin, int p1_adc, float p1_cels, int p2_adc, float p2_cels, int ch_num, int ques_bit, int scpi_error);

    uint8_t index;
	TestResult g_testResult;
	const char *name;
	int installed;
	int pin;
	int p1_adc;
	float p1_cels;
	int p2_adc;
	float p2_cels;
	int ch_num;
	int ques_bit;
	int scpi_error;

	void init();
	bool test();

	float read();

private:
    float doRead();
};

////////////////////////////////////////////////////////////////////////////////

extern TempSensor sensors[NUM_TEMP_SENSORS];

////////////////////////////////////////////////////////////////////////////////

int temperatureRead(int pin);

}
}
} // namespace eez::app::temp_sensor
