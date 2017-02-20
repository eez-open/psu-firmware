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
#include "timer.h"

namespace eez {
namespace psu {

Interval::Interval(uint32_t interval_msec)
	: interval_usec(interval_msec * 1000L)
{
	reset();
}

void Interval::reset() {
	next_tick_usec = micros() + interval_usec;
}

bool Interval::test(uint32_t tick_usec) {
	int32_t diff = tick_usec - next_tick_usec;
	if (diff > 0) {
		do {
			next_tick_usec += interval_usec;
			diff = tick_usec - next_tick_usec;
		} while (diff > 0);

		return true;
	}

	return false;
}

}
} // namespace eez::psu
