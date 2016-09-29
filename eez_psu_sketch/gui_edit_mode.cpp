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

static int tabIndex = PAGE_ID_EDIT_MODE_KEYPAD;
static data::Cursor dataCursor;
static int dataId = -1;
static data::Value editValue;
static data::Value undoValue;
static data::Value minValue;
static data::Value maxValue;
static bool is_interactive_mode = true;

////////////////////////////////////////////////////////////////////////////////

void Snapshot::takeSnapshot(data::Snapshot *snapshot) {
    if (edit_mode::isActive()) {
        editValue = edit_mode::getEditValue();

        getInfoText(infoText);

        interactiveModeSelector = isInteractiveMode() ? 0 : 1;

        step_index = edit_mode_step::getStepIndex();

        edit_mode_keypad::getText(snapshot->keypadSnapshot.text, sizeof(snapshot->keypadSnapshot.text));
    }
}

data::Value Snapshot::getData(uint8_t id) {
    if (id == DATA_ID_EDIT_VALUE) {
       return editValue;
    } else if (id == DATA_ID_EDIT_INFO) {
        return infoText;
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
    return dataId != -1;
}

bool onKeypadOk(float value) {
	return edit_mode::setValue(value);
}

void enter(int tabIndex_) {
	if (tabIndex_ != -1) {
		tabIndex = tabIndex_;
	}

    if (getActivePageId() != tabIndex) {
        if (getActivePageId() == PAGE_ID_MAIN) {
            dataCursor = g_foundWidgetAtDown.cursor;
            DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
            dataId = widget->data;
        }

        editValue = data::currentSnapshot.get(dataCursor, dataId);
        undoValue = editValue;
        minValue = data::getMin(dataCursor, dataId);
        maxValue = data::getMax(dataCursor, dataId);

        if (tabIndex == PAGE_ID_EDIT_MODE_KEYPAD) {
			numeric_keypad::Options options;

			options.editUnit = editValue.getType();

			options.min = minValue.getFloat();
			options.max = maxValue.getFloat();
			options.def = 0;

			options.flags.genericNumberKeypad = false;
			options.flags.maxButtonEnabled = true;
			options.flags.defButtonEnabled = true;
			options.flags.signButtonEnabled = true;
			options.flags.dotButtonEnabled = true;

			numeric_keypad::init(0, options, (void (*)(float))onKeypadOk, 0);
        }

        psu::enterTimeCriticalMode();

        setPage(tabIndex);
    }
}

void exit() {
    if (getActivePageId() != PAGE_ID_MAIN) {
        dataId = -1;

        psu::leaveTimeCriticalMode();

        setPage(PAGE_ID_MAIN);
    }
}

void nonInteractiveSet() {
	int16_t error;
	if (!data::set(dataCursor, dataId, editValue, &error)) {
		errorMessage(data::Value::ScpiErrorText(error));
	}
}

void nonInteractiveDiscard() {
    editValue = undoValue;
    data::set(dataCursor, dataId, undoValue, 0);
}

bool isInteractiveMode() {
    return is_interactive_mode;
}

void toggleInteractiveMode() {
    is_interactive_mode = !is_interactive_mode;
    editValue = data::currentSnapshot.get(dataCursor, dataId);
    undoValue = editValue;
}

const data::Value& getEditValue() {
    return editValue;
}

data::Value getCurrentValue(data::Snapshot snapshot) {
    return snapshot.get(dataCursor, dataId);
}

const data::Value& getMin() {
    return minValue;
}

const data::Value& getMax() {
    return maxValue;
}

data::ValueType getUnit() {
    return editValue.getType();
}

bool setValue(float value_) {
    data::Value value = data::Value(value_, getUnit());
    if (is_interactive_mode || tabIndex == PAGE_ID_EDIT_MODE_KEYPAD) {
		int16_t error;
        if (!data::set(dataCursor, dataId, value, &error)) {
			errorMessage(data::Value::ScpiErrorText(error));
            return false;
        }
    }
    editValue = value;
	return true;
}

bool isEditWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    return widgetCursor.cursor == dataCursor && widget->data == dataId;
}

void getInfoText(char *infoText) {
    Channel &channel = Channel::get(dataCursor.i);
    if (dataId == DATA_ID_CHANNEL_I_SET) {
		sprintf_P(infoText, PSTR("Set Ch%d current ["), channel.index);
		util::strcatFloat(infoText, minValue.getFloat());
		strcat_P(infoText, PSTR("-"));
		util::strcatCurrent(infoText, maxValue.getFloat());
		strcat_P(infoText, PSTR("]"));
    } else {
		sprintf_P(infoText, PSTR("Set Ch%d voltage ["), channel.index);
		util::strcatFloat(infoText, minValue.getFloat());
		strcat_P(infoText, PSTR("-"));
		util::strcatVoltage(infoText, maxValue.getFloat());
		strcat_P(infoText, PSTR("]"));
    }
}

}
}
}
} // namespace eez::psu::gui::edit_mode
