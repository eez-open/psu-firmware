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
#include "gui_edit_mode.h"
#include "gui_edit_mode_keypad.h"
#include "gui_numeric_keypad.h"

#include "sound.h"

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode_keypad {

////////////////////////////////////////////////////////////////////////////////

void getText(char *text, int count) {
    if (!numeric_keypad::getText(text, count)) {
        edit_mode::getCurrentValue(data::currentSnapshot).toText(text, count);
    }
}

}
}
}
} // namespace eez::psu::gui::edit_mode_keypad
