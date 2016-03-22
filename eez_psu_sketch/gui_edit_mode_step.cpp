/*
* EEZ PSU Firmware
* Copyright (C) 2015 Envox d.o.o.
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
#include "gui_edit_mode_step.h"

#define CONF_GUI_EDIT_MODE_STEP_THRESHOLD_PX 10

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode_step {

static float step_value = 0.1f;

static bool changed;
static int start_y;

void touch_down() {
    start_y = touch::y;
    changed = false;
}

void touch_move() {
    if (!changed) {
        int d = start_y - touch::y;
        if (abs(d) >= CONF_GUI_EDIT_MODE_STEP_THRESHOLD_PX) {
            data::Value minValue = data::getMin(edit_mode::data_cursor, edit_mode::data_id);
            float min = minValue.getFloat();
            float max = data::getMax(edit_mode::data_cursor, edit_mode::data_id).getFloat();

            float value = edit_mode::value.getFloat();

            if (d > 0) {
                value += step_value;
                if (value > max) {
                    value = max;
                }
            } else {
                value -= step_value;
                if (value < min) {
                    value = min;
                }
            }

            edit_mode::value = data::Value(value, minValue.getUnit());

            if (edit_mode::is_interactive_mode) {
                data::set(edit_mode::data_cursor, edit_mode::data_id, edit_mode::value);
            }

            changed = true;
        }
    }
}

void touch_up() {
}

}
}
}
} // namespace eez::psu::gui::edit_mode_step
