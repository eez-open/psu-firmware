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
#include "gui_edit_mode_slider.h"
#include "gui_edit_mode_keypad.h"

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode {

int tab_index = PAGE_ID_EDIT_MODE_SLIDER;
data::Cursor data_cursor;
int data_id;
data::Value value;
data::Value value_saved;
bool is_interactive_mode = true;

////////////////////////////////////////////////////////////////////////////////

void enter(const WidgetCursor &widget_cursor) {
    if (page_index != tab_index) {
        if (page_index == PAGE_ID_MAIN) {
            data_cursor = widget_cursor.cursor;
            data_id = widget_cursor.widget->data;
        }
        value = currentDataSnapshot.get(data_cursor, data_id);
        value_saved = value;

        page_index = tab_index;

        if (page_index == PAGE_ID_EDIT_MODE_SLIDER) {
            psu::enterTimeCriticalMode();
        }
        else if (page_index == PAGE_ID_EDIT_MODE_KEYPAD) {
            edit_mode_keypad::reset();
        }

        refresh_page();
    }
}

void exit() {
    if (page_index != PAGE_ID_MAIN) {
        if (page_index == ACTION_ID_EDIT_MODE_SLIDER) {
            psu::leaveTimeCriticalMode();
        }

        data_id = -1;

        page_index = PAGE_ID_MAIN;
        refresh_page();
    }
}

bool doAction(int action_id, WidgetCursor &widget_cursor) {
    if (action_id == ACTION_ID_EDIT) {
        edit_mode::enter(widget_cursor);
        return true;
    }
    
    if (action_id == ACTION_ID_EDIT_MODE_SLIDER) {
        edit_mode::tab_index = PAGE_ID_EDIT_MODE_SLIDER;
        edit_mode::enter(widget_cursor);
        return true;
    }
    
    if (action_id == ACTION_ID_EDIT_MODE_STEP) {
        edit_mode::tab_index = PAGE_ID_EDIT_MODE_STEP;
        edit_mode::enter(widget_cursor);
        return true;
    }
    
    if (action_id == ACTION_ID_EDIT_MODE_KEYPAD) {
        edit_mode::tab_index = PAGE_ID_EDIT_MODE_KEYPAD;
        edit_mode::enter(widget_cursor);
        return true;
    }

    if (action_id == ACTION_ID_EXIT) {
        if (data_id != -1) {
            edit_mode::exit();
            return true;
        }
    }

    if (action_id >= ACTION_ID_KEY_0 && action_id <= ACTION_ID_KEY_UNIT) {
        edit_mode_keypad::do_action(action_id);
        return true;
    }

    if (action_id == ACTION_ID_NON_INTERACTIVE_ENTER) {
        data::set(edit_mode::data_cursor, edit_mode::data_id, edit_mode::value);
        return true;
    }
    
    if (action_id == ACTION_ID_NON_INTERACTIVE_CANCEL) {
        edit_mode::value = edit_mode::value_saved;
        data::set(edit_mode::data_cursor, edit_mode::data_id, edit_mode::value_saved);
        return true;
    }
    
    return false;
}

}
}
}
} // namespace eez::psu::gui::edit_mode
