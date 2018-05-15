/*
* EEZ Middleware
* Copyright (C) 2017-present, Envox d.o.o.
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

namespace eez {
namespace mw {

enum Unit {
	UNIT_UNKNOWN,
	UNIT_VOLT,
	UNIT_MILLI_VOLT,
	UNIT_AMPER,
	UNIT_MILLI_AMPER,
	UNIT_WATT,
	UNIT_MILLI_WATT,
	UNIT_SECOND,
	UNIT_MILLI_SECOND,
	UNIT_CELSIUS,
	UNIT_RPM,
	UNIT_OHM,
	UNIT_KOHM,
	UNIT_MOHM
};

extern const char *g_unitNames[];

inline const char *getUnitName(Unit unit) {
	return g_unitNames[unit];
}

}
} // namespace eez::mw