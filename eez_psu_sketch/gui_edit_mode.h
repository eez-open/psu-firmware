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

#pragma once

#include "gui_internal.h"

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode {

extern int tab_index;
extern data::Cursor data_cursor;
extern int data_id;
extern data::Value value;
extern data::Value value_saved;
extern bool is_interactive_mode;

void enter(const WidgetCursor &widget_cursor);
void exit();
bool doAction(int action_id, WidgetCursor &widget_cursor);

}
}
}
} // namespace eez::psu::gui::edit_mode
