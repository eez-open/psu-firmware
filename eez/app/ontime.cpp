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

#include "eez/app/psu.h"
#include "eez/app/ontime.h"
#include "eez/app/persist_conf.h"

#define MIN_TO_MS (60L * 1000L)

namespace eez {
namespace app {
namespace ontime {

void counterToString(char *str, size_t count, uint32_t counterTime) {
	if (counterTime >= 24 * 60) {
		uint32_t d = counterTime / (24 * 60);
		counterTime -= d * (24 * 60);
		snprintf(str, count-1, "%dd %dh %dm", int(d), int(counterTime / 60), int(counterTime % 60));
	} else if (counterTime >= 60) {
		snprintf(str, count-1, "%dh %dm", int(counterTime / 60), int(counterTime % 60));
	} else {
		snprintf(str, count-1, "%dm", int(counterTime));
	}

	str[count-1] = 0;
}

Counter::Counter(int type_)
	: typeAndIsActive(type_)
	, totalTime(0)
	, lastTime(0)
	, fractionTime(0)
	, writeInterval(WRITE_ONTIME_INTERVAL * MIN_TO_MS)
{
}

void Counter::init() {
	totalTime = persist_conf::readTotalOnTime(getType());
}

int Counter::getType() {
	return typeAndIsActive & ~0x80;
}

bool Counter::isActive() {
	return (typeAndIsActive & 0x80) != 0;
}

void Counter::start() {
	if (!isActive()) {
		lastTick = millis();
		typeAndIsActive |= 0x80;
	}
}

void Counter::stop() {
	if (isActive()) {
		fractionTime += millis() - lastTick;
		typeAndIsActive &= ~0x80;
		totalTime += lastTime;
		lastTime = 0;
	}
}

void Counter::tick(uint32_t tick_usec) {
	if (isActive()) {
		uint32_t timeMS = millis() - lastTick;
		lastTick += timeMS;
		fractionTime += timeMS;
	}

	uint32_t time = fractionTime / MIN_TO_MS;
	if (time > 0) {
		lastTime += time;
		fractionTime -= time * MIN_TO_MS;
	}

	if (writeInterval.test(tick_usec)) {
		persist_conf::writeTotalOnTime(getType(), getTotalTime());
	}
}

uint32_t Counter::getTotalTime() {
	return totalTime + getLastTime();
}

uint32_t Counter::getLastTime() {
	return lastTime;
}

}
}
} // namespace eez::app::ontime
