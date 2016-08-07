/*
 * EEZ PSU Firmware
 * Copyright (C) 2016-present, Envox d.o.o.
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
#include "sound.h"

#include "gui_internal.h"
#include "gui_keypad.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {

using namespace keypad;

namespace numeric_keypad {

enum State {
    START,
    EMPTY,
    D0,
    D1,
    DOT,
    D2,
    D3
};

static State g_state;

static int g_d0;
static int g_d1;
static int g_d2;
static int g_d3;

static data::ValueType g_editUnit = data::VALUE_TYPE_NONE;

static float g_min;
static float g_max;

bool isMilli();
void toggleEditUnit();
void reset();

////////////////////////////////////////////////////////////////////////////////

void init(const char *label, data::ValueType editUnit, float min, float max, void (*ok)(float), void (*cancel)()) {
	keypad::init(label, (void (*)(char *))ok, cancel);

	g_editUnit = editUnit;
	g_min = min;
	g_max = max;

	reset();
}

void start(const char *label, data::ValueType editUnit, float min, float max, void (*ok)(float), void (*cancel)()) {
	init(label, editUnit, min, max, ok, cancel);
	showPage(PAGE_ID_NUMERIC_KEYPAD, false);
}

data::ValueType getEditUnit() {
    return g_editUnit;
}

bool getText(char *text, int count) {
    if (g_state == START) {
		*text = 0;
        return false;
    }

    int i = 0;

    if (g_state >= D0 && (g_d0 != 0 || g_state < DOT)) {
        text[i++] = g_d0 + '0';
    }

    if (g_state >= D1) {
        text[i++] = g_d1 + '0';
    }

    if (!isMilli() && g_state >= DOT) {
        text[i++] = '.';
    }

    if (g_state >= D2) {
        text[i++] = g_d2 + '0';
    }

    if (g_state >= D3) {
        text[i++] = g_d3 + '0';
    }

    text[i] = 0;

	keypad::appendCursor(text);

    if (g_editUnit == data::VALUE_TYPE_FLOAT_VOLT)
        strcat_P(text, PSTR("V"));
    else if (g_editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT)
        strcat_P(text, PSTR("mV"));
    else if (g_editUnit == data::VALUE_TYPE_FLOAT_AMPER)
        strcat_P(text, PSTR("A"));
    else if (g_editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER)
        strcat_P(text, PSTR("mA"));

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool isMilli() {
    return g_editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT || g_editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER;
}

float getValue() {
    float value = 0;

    if (g_state >= D0) {
        value = g_d0 * 1.0f;
    }

    if (g_state >= D1) {
        value = value * 10 + g_d1;
    }

    if (isMilli()) {
        if (g_state >= D2) {
            value = value * 10 + g_d2;
        }

        value /= 1000.0f;
    }
    else {
        value *= 100.0f;

        if (g_state >= D2) {
            value += g_d2 * 10;
        }

        if (g_state >= D3) {
            value += g_d3;
        }

        value /= 100.0f;
    }

    return value;
}

bool setValue(float fvalue) {
    if (isMilli()) {
        long value = (long)floor(fvalue * 1000);
        
        if (value > 999) {
            return false;
        }

        if (value >= 100) {
            g_d0 = value / 100;
            value = value % 100;
            g_d1 = value / 10;
            g_d2 = value % 10;
            g_state = D2;
        } else if (value >= 10) {
            g_d0 = value / 10;
            g_d1 = value % 10;
            g_state = D1;
        } else {
            g_d0  = value;
            g_state = D0;
        }
    } else {
        long value = (long)round(fvalue * 100);
        
        g_d3 = value % 10;
        if (g_d3 != 0) {
            g_state = D3;
            
            value /= 10;
            g_d2 = value % 10;
            
            value /= 10;
            g_d1 = value % 10;

            value /= 10;
            g_d0 = value % 10;

        } else {
            value /= 10;
            g_d2 = value % 10;
            if (g_d2 != 0) {
                g_state = D2;

                value /= 10;
                g_d1 = value % 10;

                value /= 10;
                g_d0 = value % 10;
            } else {
                value /= 10;
                g_d1 = value % 10;

                g_d0 = value / 10;
                if (g_d0 == 0) {
                    g_d0 = g_d1;
                    g_state = D0;
                } else {
                    g_state = D1;
                }
            }
        }
    }

    return true;
}

bool isValueValid() {
    if (g_state == EMPTY) {
        return false;
    }
    float value = getValue();
    return (value >= g_min && value <= g_max);
}

void digit(int d) {
    if (g_state == START || g_state == EMPTY) {
        g_d0 = d;
        g_state = D0;
        if (!isValueValid()) {
            toggleEditUnit();
        } 
        if (!isValueValid()) {
            reset();
            sound::playBeep();
        }
    }
    else if (g_state == D0) {
        g_d1 = d;
        g_state = D1;
        if (!isValueValid()) {
            g_state = D0;
            sound::playBeep();
        }
    }
    else if (g_state == DOT || (isMilli() && g_state == D1)) {
        State saved_state = g_state;
        g_d2 = d;
        g_state = D2;
        if (!isValueValid()) {
            g_state = saved_state;
            sound::playBeep();
        }
    }
    else if (g_state == D2 && !isMilli()) {
        g_d3 = d;
        g_state = D3;
        if (!isValueValid()) {
            g_state = D2;
            sound::playBeep();
        }
    }
    else {
        sound::playBeep();
    }
}

void dot() {
    if (isMilli()) {
        sound::playBeep();
    } else {
        if (g_state == START || g_state == EMPTY) {
            g_d0 = 0;
            g_d1 = 0;
            g_state = DOT;
        }
        else if (g_state == D0) {
            g_d1 = g_d0;
            g_d0 = 0;
            g_state = DOT;
        }
        else if (g_state == D1) {
            g_state = DOT;
        }
        else {
            sound::playBeep();
        }
    }
}

void toggleEditUnit() {
    if (g_editUnit == data::VALUE_TYPE_FLOAT_VOLT) {
        g_editUnit = data::VALUE_TYPE_FLOAT_MILLI_VOLT;
    }
    else if (g_editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT) {
        g_editUnit = data::VALUE_TYPE_FLOAT_VOLT;
    }
    else if (g_editUnit == data::VALUE_TYPE_FLOAT_AMPER) {
        g_editUnit = data::VALUE_TYPE_FLOAT_MILLI_AMPER;
    }
    else if (g_editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER) {
        g_editUnit = data::VALUE_TYPE_FLOAT_AMPER;
    }
}

void reset() {
    g_state = START;

	if (g_editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT) {
		g_editUnit = data::VALUE_TYPE_FLOAT_VOLT;
	} else if (g_editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER) {
		g_editUnit = data::VALUE_TYPE_FLOAT_AMPER;
	}
}

////////////////////////////////////////////////////////////////////////////////

void key(char c) {
	if (c >= '0' && c <= '9') {
		digit(c - '0');
	} else if (c == '.') {
		dot();
	}
}

void space() {
}

void caps() {
}

void back() {
    if (g_state == D3) {
        g_state = D2;
    }
    else if (g_state == D2) {
        if (isMilli()) {
            g_state = D1;
        }
        else {
            g_state = DOT;
        }
    }
    else if (g_state == DOT) {
        if (g_d0 == 0) {
            g_d0 = g_d1;
            g_state = D0;
        }
        else {
            g_state = D1;
        }
    }
    else if (g_state == D1) {
        g_state = D0;
    }
    else if (g_state == D0) {
        g_state = EMPTY;
    }
    else {
        sound::playBeep();
    }
}

void clear() {
    if (g_state != START) {
        reset();
    }
    else {
        sound::playBeep();
    }
}

void sign() {
    // not supported
    sound::playBeep();
}

void unit() {
    float value = getValue();
    data::ValueType savedEditUnit = g_editUnit;

    toggleEditUnit();

    if (g_state == START) {
        g_state = EMPTY;
    } else {
        if (g_state != EMPTY && !setValue(value)) {
            g_editUnit = savedEditUnit;
            sound::playBeep();
        }
    }
}

void ok() {
	if (g_state != START && g_state != EMPTY) {
		((void (*)(float))g_okCallback)(getValue());
		reset();
	}
	else {
		sound::playBeep();
	}

}

void cancel() {
	g_cancelCallback();
}

}
}
}
} // namespace eez::psu::gui::numeric_keypad
