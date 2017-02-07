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

#include "channel_dispatcher.h"
#include "sound.h"

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

bool isActive() {
    return getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD ||
        getActivePageId() == PAGE_ID_EDIT_MODE_STEP ||
        getActivePageId() == PAGE_ID_EDIT_MODE_SLIDER;
}

void initEditValue() {
    g_editValue = data::getEditValue(g_focusCursor, g_focusDataId);
    g_undoValue = g_editValue;
}

void enter(int tabIndex_) {
#if OPTION_ENCODER
    if (!isActive()) {
        if (!isFocusWidget(g_foundWidgetAtDown) || g_focusEditValue.getType() != data::VALUE_TYPE_NONE) {
            DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
            setFocusCursor(g_foundWidgetAtDown.cursor, widget->data);
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

    setFocusCursor(newDataCursor, newDataId);

    update();

    if (g_tabIndex == PAGE_ID_EDIT_MODE_KEYPAD) {
        edit_mode_keypad::enter(g_editValue, g_minValue, g_maxValue);
    } else {
        edit_mode_keypad::exit();
    }

    psu::enterTimeCriticalMode();

    setPage(g_tabIndex);
}

void update() {
    initEditValue();
    g_minValue = data::getMin(g_focusCursor, g_focusDataId);
    g_maxValue = data::getMax(g_focusCursor, g_focusDataId);
}

void exit() {
    edit_mode_keypad::exit();

    if (getActivePageId() != PAGE_ID_MAIN) {
        psu::leaveTimeCriticalMode();
        setPage(PAGE_ID_MAIN);
    }
}

void nonInteractiveSet() {
	int16_t error;
	if (!data::set(g_focusCursor, g_focusDataId, g_editValue, &error)) {
		errorMessage(g_focusCursor, data::Value::ScpiErrorText(error));
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

data::Value getData(const data::Cursor &cursor, uint8_t id) {
    int editInfoPartOffset = g_focusCursor.i * 6 + (g_focusDataId == DATA_ID_CHANNEL_U_EDIT ? 0 : 1) * 3;

    if (id == DATA_ID_EDIT_VALUE) {
       return edit_mode::getEditValue();
    } else if (id == DATA_ID_EDIT_INFO) {
        return data::Value(0 + editInfoPartOffset, data::VALUE_TYPE_EDIT_INFO);
    } else if (id == DATA_ID_EDIT_INFO1) {
        return data::Value(1 + editInfoPartOffset, data::VALUE_TYPE_EDIT_INFO);
    } else if (id == DATA_ID_EDIT_INFO2) {
        return data::Value(2 + editInfoPartOffset, data::VALUE_TYPE_EDIT_INFO);
    } else if (id == DATA_ID_EDIT_MODE_INTERACTIVE_MODE_SELECTOR) {
        return isInteractiveMode() ? 0 : 1;
    } else if (id == DATA_ID_EDIT_STEPS) {
        return edit_mode_step::getStepIndex();
    }

    return data::Value();
}

bool isBlinking(const data::Cursor &cursor, uint8_t id, bool &result) {
    if (id == DATA_ID_EDIT_VALUE) {
        result = !isInteractiveMode() && (edit_mode::getEditValue() != edit_mode::getCurrentValue());
        return true;
    }
    return false;
}

const data::Value& getEditValue() {
    return g_editValue;
}

data::Value getCurrentValue() {
    return data::get(g_focusCursor, g_focusDataId);
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
			errorMessage(g_focusCursor, data::Value::ScpiErrorText(error));
            return false;
        }
    }
    g_editValue = value;
	return true;
}

void getInfoText(int part, char *infoText) {
    // channel 0 u part 0
    // channel 0 u part 1
    // channel 0 u part 2
    // channel 0 i part 0
    // channel 0 i part 1
    // channel 0 i part 2
    // channel 1 u part 0
    // channel 1 u part 1
    // channel 1 u part 2
    // channel 1 i part 0
    // channel 1 i part 1
    // channel 1 i part 2
    // ...

    int iChannel = part / 6;
    
    part %= 6;
    
    int dataId;
    if (part / 3 == 0) {
        dataId = DATA_ID_CHANNEL_U_EDIT;
    } else {
        dataId = DATA_ID_CHANNEL_I_EDIT;
    }

    part %= 3;

    Channel &channel = Channel::get(iChannel);
    if (dataId == DATA_ID_CHANNEL_I_EDIT) {
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

#endif