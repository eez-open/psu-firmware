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

#include "eez/psu/psu.h"

#if OPTION_DISPLAY

#include "eez/psu/sound.h"
#include "eez/psu/channel_dispatcher.h"
#include "eez/psu/calibration.h"

#include "eez/psu/gui/psu.h"
#include "eez/psu/gui/data.h"
#include "eez/psu/gui/edit_mode.h"
#include "eez/psu/gui/edit_mode_slider.h"
#include "eez/psu/gui/edit_mode_step.h"
#include "eez/psu/gui/edit_mode_keypad.h"
#include "eez/psu/gui/numeric_keypad.h"

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

void enter(int tabIndex) {
#if OPTION_ENCODER
    if (!isActive()) {
        if (!isFocusWidget(g_foundWidgetAtDown) || g_focusEditValue.getType() != VALUE_TYPE_NONE) {
            DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
            setFocusCursor(g_foundWidgetAtDown.cursor, widget->data);
            return;
        }
    }
#endif

    if (tabIndex == -1) {
        gui::selectChannel();
        data::Cursor newDataCursor = g_foundWidgetAtDown.cursor;
        if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
            newDataCursor.i = 0;
        }
        DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
        int newDataId = widget->data;
        setFocusCursor(newDataCursor, newDataId);
        update();

        if (!isActive()) {
            pushPage(g_tabIndex);
        }
    } else {
        g_tabIndex = tabIndex;
        replacePage(g_tabIndex);
    }

    if (g_tabIndex == PAGE_ID_EDIT_MODE_KEYPAD) {
        edit_mode_keypad::enter(Channel::get(g_focusCursor.i), g_editValue, g_minValue, g_maxValue);
    } else {
        edit_mode_keypad::exit();
    }
}

void update() {
    initEditValue();
    g_minValue = data::getMin(g_focusCursor, g_focusDataId);
    g_maxValue = data::getMax(g_focusCursor, g_focusDataId);
	if (edit_mode_keypad::g_keypad) {
		edit_mode_keypad::g_keypad->m_options.editValueUnit = g_editValue.getUnit();
		if (g_editValue.isMilli()) {
			edit_mode_keypad::g_keypad->switchToMilli();
		}
	}
}

void exit() {
    edit_mode_keypad::exit();
    popPage();
}

void nonInteractiveSet() {
	int16_t error;
	if (!data::set(g_focusCursor, g_focusDataId, g_editValue, &error)) {
		errorMessage(g_focusCursor, data::MakeScpiErrorValue(error));
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

data::Value getCurrentValue() {
    return data::get(g_focusCursor, g_focusDataId);
}

const data::Value &getMin() {
    return g_minValue;
}

const data::Value &getMax() {
    return g_maxValue;
}

Unit getUnit() {
    return g_editValue.getUnit();
}

bool setValue(float value_) {
    data::Value value = MakeValue(value_, getUnit(), g_focusCursor.i);
    if (g_isInteractiveMode || g_tabIndex == PAGE_ID_EDIT_MODE_KEYPAD) {
		int16_t error;
        if (!data::set(g_focusCursor, g_focusDataId, value, &error)) {
			errorMessage(g_focusCursor, data::MakeScpiErrorValue(error));
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
                strcpy(infoText, "Set current");
            } else {
		        sprintf(infoText, "Set Ch%d current", channel.index);
            }
        } else {
            *infoText = 0;
        }

        if (part == 0) {
            strcat(infoText, " ");
        }

        if (part == 0 || part == 2) {
            strcat(infoText, "[");
		    strcatFloat(infoText, g_minValue.getFloat(), 2);
		    strcat(infoText, "-");
		    strcatCurrent(infoText, channel_dispatcher::getILimit(Channel::get(g_focusCursor.i)), 2);
		    strcat(infoText, "]");
        }
    } else {
        if (part == 0 || part == 1) {
            if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
                strcpy(infoText, "Set voltage");
            } else {
    		    sprintf(infoText, "Set Ch%d voltage", channel.index);
            }
        } else {
            *infoText = 0;
        }

        if (part == 0) {
            strcat(infoText, " ");
        }

        if (part == 0 || part == 2) {
            strcat(infoText, "[");
            strcatFloat(infoText, g_minValue.getFloat(), 2);
		    strcat(infoText, "-");
		    strcatVoltage(infoText, channel_dispatcher::getULimit(Channel::get(g_focusCursor.i)), 2);
		    strcat(infoText, "]");
        }
    }
}

}
}
}
} // namespace eez::psu::gui::edit_mode

#endif