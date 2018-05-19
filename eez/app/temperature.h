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

#include "eez/app/temp_sensor.h"

namespace eez {
namespace app {

class Channel;

/// Temperature measurement and protection.
namespace temperature {

/// Configuration data for the temperature protection.
struct ProtectionConfiguration {
    int8_t sensor;
    float delay;
    float level;
    bool state;
};

void init();
bool test();
void tick(uint32_t tick_usec);

bool isChannelSensorInstalled(Channel *channel);
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
bool getChannelSensorState(Channel *channel);
float getChannelSensorLevel(Channel *channel);
float getChannelSensorDelay(Channel *channel);
#endif

bool isAnySensorTripped(Channel *channel);

void clearChannelProtection(Channel *channel);
void disableChannelProtection(Channel *channel);

float getMaxChannelTemperature();
bool isAllowedToPowerUp();

class TempSensorTemperature {
public:
	ProtectionConfiguration prot_conf;
	float temperature;

	TempSensorTemperature(int sensorIndex);

	bool isInstalled();
	bool isTestOK();
	void tick(uint32_t tick_usec);
	bool isChannelSensor(Channel *channel);
	float measure();
	void clearProtection();
	bool isTripped();

private:
	int sensorIndex;

	bool otp_alarmed;
	uint32_t otp_alarmed_started_tick;
	bool otp_tripped;

	void set_otp_reg(bool on);
	void protection_check(uint32_t tick_usec);
    static void protection_enter(TempSensorTemperature& sensor);
	void protection_enter();
};

extern TempSensorTemperature sensors[temp_sensor::NUM_TEMP_SENSORS];

}
}
} // namespace eez::app::temperature
