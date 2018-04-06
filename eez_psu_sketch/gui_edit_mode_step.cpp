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

#if OPTION_ENCODER
#include "encoder.h"
#endif

#include "sound.h"
#include "gui_data.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_step.h"

#define CONF_GUI_EDIT_MODE_STEP_THRESHOLD_PX 5

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode_step {

using data::Value;

const Value CONF_GUI_U_STEPS[] = {
    Value(5.0f, VALUE_TYPE_FLOAT),
    Value(2.0f, VALUE_TYPE_FLOAT),
    Value(1.0f, VALUE_TYPE_FLOAT),
    Value(0.5f, VALUE_TYPE_FLOAT),
    Value(0.1f, VALUE_TYPE_FLOAT)
};

const Value CONF_GUI_I_STEPS[] = {
    Value(0.5f,  VALUE_TYPE_FLOAT),
    Value(0.25f, VALUE_TYPE_FLOAT),
    Value(0.1f,  VALUE_TYPE_FLOAT),
    Value(0.05f, VALUE_TYPE_FLOAT),
    Value(0.01f, VALUE_TYPE_FLOAT)
};

static int g_stepIndex[CH_NUM][2];

static const int DEFAULT_INDEX = 3;

static bool g_changed;
static int g_startPos;

float getStepValue() {
    if (edit_mode::getUnit() == VALUE_TYPE_FLOAT_VOLT) {
        return CONF_GUI_U_STEPS[getStepIndex()].getFloat();
    } else {
        return CONF_GUI_I_STEPS[getStepIndex()].getFloat();
    }
}

void getStepValues(const data::Value **labels, int &count) {
    if (edit_mode::getUnit() == VALUE_TYPE_FLOAT_VOLT) {
        *labels = CONF_GUI_U_STEPS;
        count = sizeof(CONF_GUI_U_STEPS) / sizeof(Value);
    } else {
        *labels = CONF_GUI_I_STEPS;
        count = sizeof(CONF_GUI_I_STEPS) / sizeof(Value);
    }
}

int getStepIndex() {
	int value;
	if (edit_mode::getUnit() == VALUE_TYPE_FLOAT_VOLT) {
		value = g_stepIndex[g_focusCursor.i][0];
	}
	else {
		value = g_stepIndex[g_focusCursor.i][1];
	}

	if (value == 0) {
		value = DEFAULT_INDEX;
	}

	return value - 1;
}

void setStepIndex(int value) {
	if (edit_mode::getUnit() == VALUE_TYPE_FLOAT_VOLT) {
		g_stepIndex[g_focusCursor.i][0] = value + 1;
	}
	else {
		g_stepIndex[g_focusCursor.i][1] = value + 1;
	}
}

void increment(int counter, bool playClick) {
    float min = edit_mode::getMin().getFloat();
    float max = edit_mode::getMax().getFloat();
    float stepValue = getStepValue();

    float value = edit_mode::getEditValue().getFloat();

    for (int i = 0; i < abs(counter); ++i) {
        if (counter > 0) {
            if (value == min) {
                value = (floorf(min / stepValue) + 1) * stepValue;
            } else {
                value += stepValue;
            }
            if (value > max) {
                value = max;
            }
        } else {
            if (value == max) {
                value = (ceilf(max / stepValue) - 1) * stepValue;
            } else {
                value -= stepValue;
            }
            if (value < min) {
                value = min;
            }
        }
    }

    if (edit_mode::setValue(value)) {
	    g_changed = true;
        if (playClick) {
            sound::playClick();
        }
	}
}

#if OPTION_ENCODER

void onEncoder(int counter) {
    encoder::enableAcceleration(false);
    increment(counter, false);
}

#endif

void test() {
    if (!g_changed) {
#if DISPLAY_ORIENTATION == DISPLAY_ORIENTATION_PORTRAIT
        int d = start_pos - touch::y;
#else
        int d = touch::x - g_startPos;
#endif
        if (abs(d) >= CONF_GUI_EDIT_MODE_STEP_THRESHOLD_PX) {
            increment(d > 0 ? 1 : -1, true);
        }
    }
}

void onTouchDown() {
#if DISPLAY_ORIENTATION == DISPLAY_ORIENTATION_PORTRAIT
    start_pos = touch::y;
#else
	g_startPos = touch::x;
#endif
    g_changed = false;
}

void onTouchMove() {
    test();
}

void onTouchUp() {
}

}
}
}
} // namespace eez::psu::gui::edit_mode_step

#endif