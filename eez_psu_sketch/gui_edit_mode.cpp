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
#include "gui_data_snapshot.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_slider.h"
#include "gui_edit_mode_step.h"
#include "gui_edit_mode_keypad.h"

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode {

static int tab_index = PAGE_ID_EDIT_MODE_SLIDER;
static data::Cursor data_cursor;
static int data_id = -1;
static data::Value edit_value;
static data::Value undo_value;
static data::Value minValue;
static data::Value maxValue;
static bool is_interactive_mode = true;

////////////////////////////////////////////////////////////////////////////////

void Snapshot::takeSnapshot() {
    if (edit_mode::isActive()) {
        editValue = edit_mode::getEditValue();

        getInfoText(infoText);

        interactiveModeSelector = isInteractiveMode() ? 0 : 1;

        step_index = edit_mode_step::getStepIndex();

        switch (edit_mode_keypad::getEditUnit()) {
        case data::UNIT_VOLT: keypadUnit = data::Value::ProgmemStr("mV"); break;
        case data::UNIT_MILLI_VOLT: keypadUnit = data::Value::ProgmemStr("V"); break;
        case data::UNIT_AMPER: keypadUnit = data::Value::ProgmemStr("mA"); break;
        default: keypadUnit = data::Value::ProgmemStr("A");
        }

        edit_mode_keypad::getText(keypadText, sizeof(keypadText));
    }
}

data::Value Snapshot::get(uint8_t id) {
    if (id == DATA_ID_EDIT_VALUE) {
       return editValue;
    } else if (id == DATA_ID_EDIT_INFO) {
        return infoText;
    } else if (id == DATA_ID_EDIT_UNIT) {
        return keypadUnit;
    } else if (id == DATA_ID_EDIT_MODE_INTERACTIVE_MODE_SELECTOR) {
        return interactiveModeSelector;
    } else if (id == DATA_ID_EDIT_STEPS) {
        return step_index;
    }

    return data::Value();
}

bool Snapshot::isBlinking(data::Snapshot& snapshot, uint8_t id, bool &result) {
    if (id == DATA_ID_EDIT_VALUE) {
        result = (interactiveModeSelector == 1) && (editValue != edit_mode::getCurrentValue(snapshot));
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool isActive() {
    return data_id != -1;
}

void enter(const WidgetCursor &widgetCursor) {
    if (getActivePage() != tab_index) {
        if (getActivePage() == PAGE_ID_MAIN) {
            data_cursor = widgetCursor.cursor;
            data_id = widgetCursor.widget->data;
        }

        edit_value = data::currentSnapshot.get(data_cursor, data_id);
        undo_value = edit_value;
        minValue = data::getMin(data_cursor, data_id);
        maxValue = data::getMax(data_cursor, data_id);

        if (tab_index == PAGE_ID_EDIT_MODE_SLIDER) {
            psu::enterTimeCriticalMode();
        }
        else if (tab_index == PAGE_ID_EDIT_MODE_KEYPAD) {
            edit_mode_keypad::reset();
        }

        showPage(tab_index);
    }
}

void exit() {
    if (getActivePage() != PAGE_ID_MAIN) {
        if (getActivePage() == ACTION_ID_EDIT_MODE_SLIDER) {
            psu::leaveTimeCriticalMode();
        }

        data_id = -1;

        showPage(PAGE_ID_MAIN);
    }
}

bool doAction(int action_id, WidgetCursor &widgetCursor) {
    if (action_id == ACTION_ID_EDIT) {
        enter(widgetCursor);
        return true;
    }
    
    if (action_id == ACTION_ID_EDIT_MODE_SLIDER) {
        tab_index = PAGE_ID_EDIT_MODE_SLIDER;
        enter(widgetCursor);
        return true;
    }
    
    if (action_id == ACTION_ID_EDIT_MODE_STEP) {
        tab_index = PAGE_ID_EDIT_MODE_STEP;
        enter(widgetCursor);
        return true;
    }
    
    if (action_id == ACTION_ID_EDIT_MODE_KEYPAD) {
        tab_index = PAGE_ID_EDIT_MODE_KEYPAD;
        enter(widgetCursor);
        return true;
    }

    if (action_id == ACTION_ID_EXIT) {
        if (isActive()) {
            exit();
            return true;
        }
    }

    if (action_id >= ACTION_ID_KEY_0 && action_id <= ACTION_ID_KEY_UNIT) {
        edit_mode_keypad::doAction(action_id);
        return true;
    }

    if (action_id == ACTION_ID_NON_INTERACTIVE_ENTER) {
        data::set(data_cursor, data_id, edit_value);
        return true;
    }
    
    if (action_id == ACTION_ID_NON_INTERACTIVE_CANCEL) {
        edit_value = undo_value;
        data::set(data_cursor, data_id, undo_value);
        return true;
    }
    
    return false;
}

bool isInteractiveMode() {
    return is_interactive_mode;
}

void toggleInteractiveMode() {
    is_interactive_mode = !is_interactive_mode;
    edit_value = data::currentSnapshot.get(data_cursor, data_id);
    undo_value = edit_value;
}

const data::Value& getEditValue() {
    return edit_value;
}

data::Value getCurrentValue(data::Snapshot snapshot) {
    return snapshot.get(data_cursor, data_id);
}

const data::Value& getMin() {
    return minValue;
}

const data::Value& getMax() {
    return maxValue;
}

data::Unit getUnit() {
    return edit_value.getUnit();
}

void setValue(float value_) {
    edit_value = data::Value(value_, getUnit());

    if (is_interactive_mode) {
        data::set(data_cursor, data_id, edit_value);
    }
}

bool isEditWidget(const WidgetCursor &widgetCursor) {
    return widgetCursor.cursor == data_cursor && widgetCursor.widget->data == data_id;
}

void getInfoText(char *infoText) {
    Channel &channel = Channel::get(data_cursor.iChannel);
    if (data_id == DATA_ID_VOLT) {
        sprintf_P(infoText, PSTR("Set Ch%d voltage [%d-%d V]"), channel.index, (int)minValue.getFloat(), (int)maxValue.getFloat());
    } else {
        sprintf_P(infoText, PSTR("Set Ch%d current [%d-%d A]"), channel.index, (int)minValue.getFloat(), (int)maxValue.getFloat());
    }
}

}
}
}
} // namespace eez::psu::gui::edit_mode
