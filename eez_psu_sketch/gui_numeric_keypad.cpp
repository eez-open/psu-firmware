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
    D3,

	BEFORE_DOT,
	AFTER_DOT
};

static State g_state;

static int g_d0;
static int g_d1;
static int g_d2;
static int g_d3;

static Options g_options;

bool isMilli();
void toggleEditUnit();
void reset();

////////////////////////////////////////////////////////////////////////////////

void init(const char *label, Options &options, void (*ok)(float), void (*cancel)()) {
	keypad::init(label, (void (*)(char *))ok, cancel);

	g_options = options;

	if (g_options.flags.genericNumberKeypad) {
		g_maxChars = 16;
	}

	reset();
}

void start(const char *label, Options &options, void (*ok)(float), void (*cancel)()) {
	init(label, options, ok, cancel);
	pushPage(PAGE_ID_NUMERIC_KEYPAD);
}

data::ValueType getEditUnit() {
    return g_options.editUnit;
}

char getDotSign() {
	if (g_options.editUnit == data::VALUE_TYPE_TIME_ZONE) {
		return ':';
	}
	return '.';
}

bool getText(char *text, int count) {
	if (g_options.flags.genericNumberKeypad) {
		strcpy(text, g_keypadText);
	} else {
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
			text[i++] = getDotSign();
		}

		if (g_state >= D2) {
			text[i++] = g_d2 + '0';
		}

		if (g_state >= D3) {
			text[i++] = g_d3 + '0';
		}

		text[i] = 0;
	}

	appendCursor(text);

	// TODO move this to data::Value
	if (g_options.editUnit == data::VALUE_TYPE_FLOAT_VOLT) {
		strcat_P(text, PSTR("V"));
	} else if (g_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT) {
		strcat_P(text, PSTR("mV"));
	} else if (g_options.editUnit == data::VALUE_TYPE_FLOAT_AMPER) {
		strcat_P(text, PSTR("A"));
	} else if (g_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER) {
		strcat_P(text, PSTR("mA"));
	} else if (g_options.editUnit == data::VALUE_TYPE_FLOAT_WATT) {
		strcat_P(text, PSTR("W"));
	} else if (g_options.editUnit == data::VALUE_TYPE_FLOAT_SECOND) {
		strcat_P(text, PSTR("s"));
	} else if (g_options.editUnit == data::VALUE_TYPE_FLOAT_CELSIUS) {
		strcat_P(text, PSTR("oC"));
	}

	return true;
}

data::Value getData(uint8_t id) {
	if (id == DATA_ID_KEYPAD_MAX_ENABLED) {
		return data::Value(g_options.flags.maxButtonEnabled ? 1 : 0);
	} else if (id == DATA_ID_KEYPAD_DEF_ENABLED) {
		return data::Value(g_options.flags.defButtonEnabled ? 1 : 0);
	} else if (id == DATA_ID_KEYPAD_DOT_ENABLED) {
		return data::Value(g_options.flags.dotButtonEnabled ? 1 : 0);
	} else if (id == DATA_ID_KEYPAD_SIGN_ENABLED) {
		return data::Value(g_options.flags.signButtonEnabled ? 1 : 0);
	} else if (id == DATA_ID_KEYPAD_UNIT_ENABLED) {
		return data::Value(
			g_options.editUnit == data::VALUE_TYPE_FLOAT_VOLT ||
			g_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT ||
			g_options.editUnit == data::VALUE_TYPE_FLOAT_AMPER ||
			g_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER ? 1 : 0);
	}

	return data::Value();
}

////////////////////////////////////////////////////////////////////////////////

bool isMilli() {
	return g_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT || g_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER;
}

void switchToMilli() {
	if (g_options.editUnit == data::VALUE_TYPE_FLOAT_VOLT) {
		g_options.editUnit = data::VALUE_TYPE_FLOAT_MILLI_VOLT;
	}
	else if (g_options.editUnit == data::VALUE_TYPE_FLOAT_AMPER) {
		g_options.editUnit = data::VALUE_TYPE_FLOAT_MILLI_AMPER;
	}
}

float getValue() {
	if (g_options.flags.genericNumberKeypad) {
		const char *p = g_keypadText;

		int a = 0;
		float b = 0;
		int sign = 1;

		if (*p == '-') {
			sign = -1;
			++p;
		} else if (*p == '+') {
			++p;
		}

		while (*p && *p != getDotSign()) {
			a = a * 10 + (*p - '0');
			++p;
		}

		if (*p) {
			const char *q = p + strlen(p) - 1;
			while (q != p) {
				b = (b + (*q - '0')) / 10;
				--q;
			}
		}

		float value = sign * (a + b);

		if (isMilli()) {
			value /= 1000.0f;
		}

		return value;
	} else {
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
}

bool setValue(float fvalue) {
	if (g_options.flags.genericNumberKeypad) {
		return false;
	} else {
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
}

bool isValueValid() {
	if (g_state == EMPTY) {
		return false;
	}
	float value = getValue();
	return (value >= g_options.min && value <= g_options.max);
}

void digit(int d) {
	if (g_options.flags.genericNumberKeypad) {
		if (g_state == EMPTY) {
			g_state = BEFORE_DOT;
			if (g_options.editUnit == data::VALUE_TYPE_TIME_ZONE) {
				if (strlen(g_keypadText) == 0) {
					appendChar('+');
				}
			}
		}
		appendChar(d + '0');
	} else {
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
}

void dot() {
	if (g_options.flags.genericNumberKeypad) {
		if (!g_options.flags.dotButtonEnabled) {
			return;
		}

		if (g_state == EMPTY) {
			if (g_options.editUnit == data::VALUE_TYPE_TIME_ZONE) {
				if (strlen(g_keypadText) == 0) {
					appendChar('+');
				}
			}
			appendChar('0');
			g_state = BEFORE_DOT;
		}

		if (g_state == BEFORE_DOT) {
			appendChar(getDotSign());
			g_state = AFTER_DOT;
		} else {
			sound::playBeep();
		}
	} else {
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
}

void toggleEditUnit() {
	if (g_options.editUnit == data::VALUE_TYPE_FLOAT_VOLT) {
		g_options.editUnit = data::VALUE_TYPE_FLOAT_MILLI_VOLT;
	}
	else if (g_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT) {
		g_options.editUnit = data::VALUE_TYPE_FLOAT_VOLT;
	}
	else if (g_options.editUnit == data::VALUE_TYPE_FLOAT_AMPER) {
		g_options.editUnit = data::VALUE_TYPE_FLOAT_MILLI_AMPER;
	}
	else if (g_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER) {
		g_options.editUnit = data::VALUE_TYPE_FLOAT_AMPER;
	}
}

void reset() {
	if (g_options.flags.genericNumberKeypad) {
		g_state = EMPTY;
		g_state = EMPTY;
		g_keypadText[0] = 0;
	} else {
		g_state = START;

		if (g_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT) {
			g_options.editUnit = data::VALUE_TYPE_FLOAT_VOLT;
		} else if (g_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER) {
			g_options.editUnit = data::VALUE_TYPE_FLOAT_AMPER;
		}
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
	// DO NOTHING
}

void caps() {
	// DO NOTHING
}

void back() {
	if (g_options.flags.genericNumberKeypad) {
		int n = strlen(g_keypadText);
		if (n > 0) {
			if (g_keypadText[n - 1] == getDotSign()) {
				g_state = BEFORE_DOT;
			}
			g_keypadText[n - 1] = 0;
			if (n - 1 == 1) {
				if (g_keypadText[0] == '+' || g_keypadText[0] == '-') {
					g_state = EMPTY;
				}
			} else if (n - 1 == 0) {
				g_state = EMPTY;
			}
		} else {
			sound::playBeep();
		}
	} else {
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
}

void clear() {
	if (g_state != START) {
		reset();
	} else {
		sound::playBeep();
	}
}

void sign() {
	if (g_options.flags.signButtonEnabled) {
		if (g_options.flags.genericNumberKeypad && g_options.editUnit == data::VALUE_TYPE_TIME_ZONE) {
			if (g_keypadText[0] == 0) {
				g_keypadText[0] = '-';
				g_keypadText[1] = 0;
			} else if (g_keypadText[0] == '-') {
				g_keypadText[0] = '+';
			} else {
				g_keypadText[0] = '-';
			}
		} else {
			// not supported
			sound::playBeep();
		}
	}
}

void unit() {
	if (g_options.flags.genericNumberKeypad) {
		toggleEditUnit();
	} else {
		float value = getValue();
		data::ValueType savedEditUnit = g_options.editUnit;

		toggleEditUnit();

		if (g_state == START) {
			g_state = EMPTY;
		} else {
			if (g_state != EMPTY && !setValue(value)) {
				g_options.editUnit = savedEditUnit;
				sound::playBeep();
			}
		}
	}
}

void setMaxValue() {
	if (g_options.flags.maxButtonEnabled) {
		((void (*)(float))g_okCallback)(g_options.max);
	}
}

void setDefaultValue() {
	if (g_options.flags.defButtonEnabled) {
		((void (*)(float))g_okCallback)(g_options.def);
	}
}

void ok() {
	if (g_state != START && g_state != EMPTY) {
		if (g_options.flags.genericNumberKeypad) {
			float value = getValue();

			if (value < g_options.min) {
				errorMessage(data::Value::LessThenMinMessage(g_options.min, g_options.editUnit));
			} else if (value > g_options.max) {
				errorMessage(data::Value::GreaterThenMaxMessage(g_options.max, g_options.editUnit));
			} else {
				((void (*)(float))g_okCallback)(value);
			}
		} else {
			if (((bool (*)(float))g_okCallback)(getValue())) {
				reset();
			}
		}
	}
	else {
		sound::playBeep();
	}
}

void cancel() {
	if (g_cancelCallback) {
		g_cancelCallback();
	} else {
		popPage();
	}
}

}
}
}
} // namespace eez::psu::gui::numeric_keypad
