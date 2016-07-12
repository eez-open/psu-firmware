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
#include "fan.h"

namespace eez {
namespace psu {
namespace temperature {

#define TEMP_SENSOR(NAME, INSTALLED, PIN, CAL_POINTS, CH_NUM, QUES_REG_BIT) \
	TempSensorTemperature(temp_sensor::NAME),

TempSensorTemperature sensors[temp_sensor::NUM_TEMP_SENSORS] = {
	TEMP_SENSORS
};

#undef TEMP_SENSOR

////////////////////////////////////////////////////////////////////////////////

static unsigned long last_measured_tick;

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

		fan::tick(tick_usec);
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