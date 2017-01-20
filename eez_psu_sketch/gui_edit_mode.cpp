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
#include "channel_dispatcher.h"
#include "sound.h"

#include "gui_data_snapshot.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_slider.h"
#include "gui_edit_mode_step.h"
#include "gui_edit_mode_keypad.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode {

static int g_tabIndex = PAGE_ID_EDIT_MODE_KEYPAD;

static data::Value g_editValue;
static data::Value g_undoValue;
static data::Value g_minValue;
static data::Value g_maxValue;
static bool g_isInteractiveMode = true;

////////////////////////////////////////////////////////////////////////////////

void Snapshot::takeSnapshot(data::Snapshot *snapshot) {
    if (edit_mode::isActive()) {
        editValue = edit_mode::getEditValue();

        interactiveModeSelector = isInteractiveMode() ? 0 : 1;

        step_index = edit_mode_step::getStepIndex();

        edit_mode_keypad::getText(snapshot->keypadSnapshot.text, sizeof(snapshot->keypadSnapshot.text));
    }
}

data::Value Snapshot::getData(uint8_t id) {
    if (id == DATA_ID_EDIT_VALUE) {
       return editValue;
    } else if (id == DATA_ID_EDIT_INFO) {
        return data::Value(0, data::VALUE_TYPE_EDIT_INFO);
    } else if (id == DATA_ID_EDIT_INFO1) {
        return data::Value(1, data::VALUE_TYPE_EDIT_INFO);
    } else if (id == DATA_ID_EDIT_INFO2) {
        return data::Value(2, data::VALUE_TYPE_EDIT_INFO);
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
    return getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD ||
        getActivePageId() == PAGE_ID_EDIT_MODE_STEP ||
        getActivePageId() == PAGE_ID_EDIT_MODE_SLIDER;
}

bool onKeypadOk(float value) {
	return edit_mode::setValue(value);
}

void initEditValue() {
    g_editValue = data::currentSnapshot.getEditValue(g_focusCursor, g_focusDataId);
    g_undoValue = g_editValue;
}

void enter(int tabIndex_) {
#if OPTION_ENCODER
    if (getActivePageId() == PAGE_ID_MAIN) {
        if (!isFocusWidget(g_foundWidgetAtDown)) {
            g_focusCursor = g_foundWidgetAtDown.cursor;
            DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
            g_focusDataId = widget->data;
            return;
        }
    }
#endif

    gui::selectChannel();

    if (tabIndex_ != -1) {
		g_tabIndex = tabIndex_;
	}

    data::Cursor newDataCursor;
    int newDataId;
    if (tabIndex_ == -1) {
        newDataCursor = g_foundWidgetAtDown.cursor;
        DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
        newDataId = widget->data;
    } else {
        newDataCursor = g_focusCursor;
        newDataId = g_focusDataId;
    }

    if (getActivePageId() != g_tabIndex || g_focusDataId != newDataId || g_focusCursor != newDataCursor) {
        if (getActivePageId() == g_tabIndex) {
            if (numeric_keypad::isEditing()) {
                sound::playBeep();
                return;
            }
        }

        g_focusCursor = newDataCursor;
        g_focusDataId = newDataId;

        initEditValue();

        g_minValue = data::getMin(g_focusCursor, g_focusDataId);
        g_maxValue = data::getMax(g_focusCursor, g_focusDataId);

        if (g_tabIndex == PAGE_ID_EDIT_MODE_KEYPAD) {
			numeric_keypad::Options options;

			options.editUnit = g_editValue.getType();

			options.min = g_minValue.getFloat();
			options.max = g_maxValue.getFloat();
			options.def = 0;

			options.flags.genericNumberKeypad = false;
			options.flags.maxButtonEnabled = true;
			options.flags.defButtonEnabled = true;
			options.flags.signButtonEnabled = true;
			options.flags.dotButtonEnabled = true;

			numeric_keypad::init(0, options, (void (*)(float))onKeypadOk, 0);
        }

        psu::enterTimeCriticalMode();

        setPage(g_tabIndex);
    }
}

void exit() {
    if (getActivePageId() != PAGE_ID_MAIN) {
        psu::leaveTimeCriticalMode();
        setPage(PAGE_ID_MAIN);
    }
}

void nonInteractiveSet() {
	int16_t error;
	if (!data::set(g_focusCursor, g_focusDataId, g_editValue, &error)) {
		errorMessage(data::Value::ScpiErrorText(error));
	}
}

void nonInteractiveDiscard() {
    g_editValue = g_undoValue;
    data::set(g_focusCursor, g_focusDataId, g_undoValue, 0);
}

bool isInteractiveMode() {
    return g_isInteractiveMode;
}

void toggleInteractiveMode() {
    g_isInteractiveMode = !g_isInteractiveMode;
    initEditValue();
}

const data::Value& getEditValue() {
    return g_editValue;
}

data::Value getCurrentValue(data::Snapshot snapshot) {
    return snapshot.get(g_focusCursor, g_focusDataId);
}

const data::Value &getMin() {
    return g_minValue;
}

const data::Value &getMax() {
    return g_maxValue;
}

data::ValueType getUnit() {
    return g_editValue.getType();
}

bool setValue(float value_) {
    data::Value value = data::Value(value_, getUnit());
    if (g_isInteractiveMode || g_tabIndex == PAGE_ID_EDIT_MODE_KEYPAD) {
		int16_t error;
        if (!data::set(g_focusCursor, g_focusDataId, value, &error)) {
			errorMessage(data::Value::ScpiErrorText(error));
            return false;
        }
    }
    g_editValue = value;
	return true;
}

void getInfoText(int part, char *infoText) {
    Channel &channel = Channel::get(g_focusCursor.i);
    if (g_focusDataId == DATA_ID_CHANNEL_I_SET) {
        if (part == 0 || part == 1) {
            if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
                strcpy_P(infoText, PSTR("Set current"));
            } else {
		        sprintf_P(infoText, PSTR("Set Ch%d current"), channel.index);
            }
        } else {
            *infoText = 0;
        }

        if (part == 0) {
            strcat_P(infoText, PSTR(" "));
        }

        if (part == 0 || part == 2) {
            strcat_P(infoText, PSTR("["));
		    util::strcatFloat(infoText, g_minValue.getFloat());
		    strcat_P(infoText, PSTR("-"));
		    util::strcatCurrent(infoText, channel_dispatcher::getILimit(Channel::get(g_focusCursor.i)));
		    strcat_P(infoText, PSTR("]"));
        }
    } else {
        if (part == 0 || part == 1) {
            if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
                strcpy_P(infoText, PSTR("Set voltage"));
            } else {
    		    sprintf_P(infoText, PSTR("Set Ch%d voltage"), channel.index);
            }
        } else {
            *infoText = 0;
        }

        if (part == 0) {
            strcat_P(infoText, PSTR(" "));
        }

        if (part == 0 || part == 2) {
            strcat_P(infoText, PSTR("["));
            util::strcatFloat(infoText, g_minValue.getFloat());
		    strcat_P(infoText, PSTR("-"));
		    util::strcatVoltage(infoText, channel_dispatcher::getULimit(Channel::get(g_focusCursor.i)));
		    strcat_P(infoText, PSTR("]"));
        }
    }
}

}
}
}
} // namespace eez::psu::gui::edit_mode
