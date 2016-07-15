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
#include "watchdog.h"
#include "timer.h"

namespace eez {
namespace psu {
namespace watchdog {

static Interval watchdogInterval(WATCHDOG_INTERVAL);

void init() {
}

void tick(unsigned long tick_usec) {
	if (watchdogInterval.test(tick_usec)) {
		digitalWrite(WATCHDOG, HIGH);
		delayMicroseconds(1);
		digitalWrite(WATCHDOG, LOW);
	}
}

}
}
} // namespace eez::psu::watchdog
