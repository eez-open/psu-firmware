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

Counter::Counter(int type_)
	: type(type_)
	, lastTime(0)
	, isActive(false)
	, writeInterval(WRITE_ONTIME_INTERVAL * MIN_TO_MS)
	, fractionTime(0)
{
}

void Counter::init() {
	totalTime = persist_conf::readTotalOnTime(type);
}

void Counter::start() {
	if (!isActive) {
		lastTick = millis();
		isActive = true;
	}
}

void Counter::stop() {
	if (isActive) {
		fractionTime += millis() - lastTick;
		isActive = false;
		lastTime = 0;
	}
}

void Counter::tick(unsigned long tick_usec) {
	if (isActive) {
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
		persist_conf::writeTotalOnTime(type, getTotalTime());
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
