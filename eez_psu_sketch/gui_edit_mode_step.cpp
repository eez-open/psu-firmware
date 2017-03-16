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
    Value(0.5f, VALUE_TYPE_FLOAT),
    Value(0.25f, VALUE_TYPE_FLOAT),
    Value(0.1f, VALUE_TYPE_FLOAT),
    Value(0.05f, VALUE_TYPE_FLOAT),
    Value(0.01f, VALUE_TYPE_FLOAT)
};

static int step_index = 2;

static bool changed;
static int start_pos;

float getStepValue() {
    if (edit_mode::getUnit() == VALUE_TYPE_FLOAT_VOLT) {
        return CONF_GUI_U_STEPS[step_index].getFloat();
    } else {
        return CONF_GUI_I_STEPS[step_index].getFloat();
    }
}

int getStepIndex() {
    return step_index;
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

void setStepIndex(int value) {
    step_index = value;
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
	    changed = true;
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
    if (!changed) {
#if DISPLAY_ORIENTATION == DISPLAY_ORIENTATION_PORTRAIT
        int d = start_pos - touch::y;
#else
        int d = touch::x - start_pos;
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
	start_pos = touch::x;
#endif
    changed = false;
}

void onTouchMove() {
    test();
}

void onTouchUp() {
    test();
}

}
}
}
} // namespace eez::psu::gui::edit_mode_step

#endif