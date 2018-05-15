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

#include "eez/mw/gui/data.h"

namespace eez {
namespace psu {
namespace gui {

class NumericKeypad;

namespace edit_mode_keypad {

void enter(Channel &channel, const eez::mw::gui::data::Value& editValue, const eez::mw::gui::data::Value& minValue, eez::mw::gui::data::Value& maxValue);
void exit();

extern NumericKeypad *g_keypad;

}
}
}
} // namespace eez::psu::gui::edit_mode_keypad
