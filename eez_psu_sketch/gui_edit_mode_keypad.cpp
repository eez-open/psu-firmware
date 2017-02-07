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

#if OPTION_DISPLAY

#include "gui_edit_mode.h"
#include "gui_edit_mode_keypad.h"
#include "gui_numeric_keypad.h"

#include "sound.h"

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode_keypad {

NumericKeypad *g_keypad;

////////////////////////////////////////////////////////////////////////////////

bool onKeypadOk(float value) {
	return edit_mode::setValue(value);
}

void enter(const data::Value& editValue, const data::Value& minValue, data::Value& maxValue) {
    g_keypad = new NumericKeypad();

	NumericKeypadOptions options;

	options.editUnit = editValue.getType();

	options.min = minValue.getFloat();
	options.max = maxValue.getFloat();
	options.def = 0;

	options.flags.genericNumberKeypad = false;
	options.flags.maxButtonEnabled = true;
	options.flags.defButtonEnabled = true;
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	g_keypad->init(0, data::Value(), options, (void (*)(float))onKeypadOk, 0);
}

void exit() {
    if (g_keypad) {
        delete g_keypad;
        g_keypad = 0;
    }
}

}
}
}
} // namespace eez::psu::gui::edit_mode_keypad

#endif