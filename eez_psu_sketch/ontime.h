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
 
#pragma once

#include "timer.h"

namespace eez {
namespace psu {
namespace ontime {

enum {
	ON_TIME_COUNTER_POWER,
	ON_TIME_COUNTER_CH1,
	ON_TIME_COUNTER_CH2
};

class Counter {
public:
	Counter(int type);

	int getType();
	bool isActive();

	void start();
	void stop();

	void init();
	void tick(unsigned long tick_usec);

	unsigned long getTotalTime();
	unsigned long getLastTime();

private:
	uint8_t typeAndIsActive;
	unsigned long totalTime;
	unsigned long lastTime;
	unsigned long lastTick;
	unsigned long fractionTime;
	Interval writeInterval;
};

}
}
} // namespace eez::psu::ontime
