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

#include "psu.h"
#include "ontime.h"
#include "persist_conf.h"

#define MIN_TO_MS (60L * 1000L)

namespace eez {
namespace psu {
namespace ontime {

void counterToString(char *str, size_t count, uint32_t counterTime) {
	if (counterTime >= 24 * 60) {
		uint32_t d = counterTime / (24 * 60);
		counterTime -= d * (24 * 60);
		snprintf_P(str, count-1, PSTR("%dd %dh %dm"), int(d), int(counterTime / 60), int(counterTime % 60));
	} else if (counterTime >= 60) {
		snprintf_P(str, count-1, PSTR("%dh %dm"), int(counterTime / 60), int(counterTime % 60));
	} else {
		snprintf_P(str, count-1, PSTR("%dm"), int(counterTime));
	}

	str[count-1] = 0;
}

Counter::Counter(int type_)
	: typeAndIsActive(type_)
	, lastTime(0)
	, writeInterval(WRITE_ONTIME_INTERVAL * MIN_TO_MS)
	, fractionTime(0)
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

void Counter::tick(unsigned long tick_usec) {
	if (isActive()) {
		unsigned long timeMS = millis() - lastTick;
		lastTick += timeMS;
		fractionTime += timeMS;
	}

	unsigned long time = fractionTime / MIN_TO_MS;
	if (time > 0) {
		lastTime += time;
		fractionTime -= time * MIN_TO_MS;
	}

	if (writeInterval.test(tick_usec)) {
		persist_conf::writeTotalOnTime(getType(), getTotalTime());
	}
}

unsigned long Counter::getTotalTime() {
	return totalTime + getLastTime();
}

unsigned long Counter::getLastTime() {
	return lastTime;
}

}
}
} // namespace eez::psu::ontime
