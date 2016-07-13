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
#include "serial_psu.h"
#include "ethernet.h"
#include "persist_conf.h"
#include "sound.h"
#include "temperature.h"

namespace eez {
namespace psu {
namespace temperature {

#define TEMP_SENSOR(NAME, INSTALLED, PIN, CAL_POINTS, CH_NUM, QUES_REG_BIT, SCPI_ERROR) \
	TempSensorTemperature(temp_sensor::NAME),

TempSensorTemperature sensors[temp_sensor::NUM_TEMP_SENSORS] = {
	TEMP_SENSORS
};

#undef TEMP_SENSOR

////////////////////////////////////////////////////////////////////////////////

static unsigned long last_measured_tick;
static float last_max_channel_temperature;
static unsigned long max_temp_start_tick;
static float force_power_down = false;

bool init() {
	return test();
}

bool test() {
	bool success = true;

	for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
		success &= temp_sensor::sensors[i].test();
	}

	return success;
}

void tick(unsigned long tick_usec) {
	if (tick_usec - last_measured_tick >= TEMP_SENSOR_READ_EVERY_MS * 1000L) {
		last_measured_tick = tick_usec;

		for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
			sensors[i].tick(tick_usec);
		}

		// find max. channel temperature
		float max_channel_temperature = FAN_MIN_TEMP - 1;

		for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
			temp_sensor::TempSensor &sensor = temp_sensor::sensors[i];
			if (sensor.ch_num >= 0) {
				if (sensor.test_result == psu::TEST_OK) {
					temperature::TempSensorTemperature &sensorTemperature = temperature::sensors[i];
					if (sensorTemperature.temperature > max_channel_temperature) {
						max_channel_temperature = sensorTemperature.temperature;
					}
				} else if (sensor.test_result == psu::TEST_FAILED) {
					psu::setCurrentMaxLimit(FAN_ERR_CURRENT);
				}
			}
		}

		// check if max_channel_temperature is too high
		if (max_channel_temperature > FAN_MAX_TEMP) {
			if (last_max_channel_temperature <= FAN_MAX_TEMP) {
				max_temp_start_tick = tick_usec;
			}

			if (tick_usec - max_temp_start_tick > FAN_MAX_TEMP_DELAY * 1000000L) {
				// turn off power
				force_power_down = true;
				psu::changePowerState(false);
			}
		} else if (max_channel_temperature <= FAN_MAX_TEMP - FAN_MAX_TEMP_DROP) {
			force_power_down = false;
		}

		last_max_channel_temperature = max_channel_temperature;
	}
}

bool isChannelTripped(Channel *channel) {
	for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
		if (sensors[i].isChannelTripped(channel)) {
			return true;
		}
	}

	return false;
}

float getMaxChannelTemperature() {
	return last_max_channel_temperature;
}

bool isAllowedToPowerUp() {
#if OPTION_MAIN_TEMP_SENSOR
	if (temperature::sensors[temp_sensor::MAIN].isTripped()) return false;
#endif

	return force_power_down;
}

////////////////////////////////////////////////////////////////////////////////

TempSensorTemperature::TempSensorTemperature(int sensorIndex_)
	: sensorIndex(sensorIndex_)
{
}

void TempSensorTemperature::tick(unsigned long tick_usec) {
	if (temp_sensor::sensors[sensorIndex].installed) {
		temperature = temp_sensor::sensors[sensorIndex].read();
		protection_check(tick_usec);
	}
}

bool TempSensorTemperature::isChannelTripped(Channel *channel) {
	if (
		temp_sensor::sensors[sensorIndex].installed && (
			temp_sensor::sensors[sensorIndex].ch_num < 0 ||
			channel->index == temp_sensor::sensors[sensorIndex].ch_num + 1
		)
	) {
		return otp_tripped;
	}

    return false;
}

float TempSensorTemperature::measure() {
    temperature = temp_sensor::sensors[sensorIndex].read();
    return temperature;
}

void TempSensorTemperature::clearProtection() {
    otp_tripped = false;
    set_otp_reg(false);
}

bool TempSensorTemperature::isTripped() {
    return otp_tripped;
}

void TempSensorTemperature::set_otp_reg(bool on) {
	if (temp_sensor::sensors[sensorIndex].ch_num >= 0) {
		Channel::get(temp_sensor::sensors[sensorIndex].ch_num).setQuesBits(temp_sensor::sensors[sensorIndex].ques_bit, on);
	} else {
		psu::setQuesBits(temp_sensor::sensors[sensorIndex].ques_bit, on);
	}
}

void TempSensorTemperature::protection_check(unsigned long tick_usec) {
	if (temp_sensor::sensors[sensorIndex].installed) {
		if (otp_tripped) {
			return;
		}

		if (prot_conf.state && temperature >= prot_conf.level) {
			float delay = prot_conf.delay;
			if (delay > 0) {
				if (otp_alarmed) {
					if (tick_usec - otp_alarmed_started_tick >= delay * 1000000UL) {
						otp_alarmed = 0;
						protection_enter();
					}
				} else {
					otp_alarmed = 1;
					otp_alarmed_started_tick = tick_usec;
				}
			} else {
				protection_enter();
			}
		} else {
			otp_alarmed = 0;
		}
	}
}

void TempSensorTemperature::protection_enter() {
    otp_tripped = true;

	if (temp_sensor::sensors[sensorIndex].ch_num >= 0) {
		Channel::get(temp_sensor::sensors[sensorIndex].ch_num).outputEnable(false);
	} else {
		for (int i = 0; i < CH_NUM; ++i) {
			Channel::get(i).outputEnable(false);
		}
		psu::powerDownBySensor();
	}
	
    set_otp_reg(true);

    sound::playBeep();
}

}
}
} // namespace eez::psu::temperature