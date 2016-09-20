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

#include <ctype.h>

#include "psu.h"
#include "sound.h"

#include "gui_internal.h"
#include "gui_data_snapshot.h"
#include "gui_keypad.h"
#include "gui_numeric_keypad.h"

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#define CONF_GUI_KEYPAD_CURSOR_BLINK_TIME 750000UL
#define CONF_GUI_KEYPAD_CURSOR_ON "|"
#define CONF_GUI_KEYPAD_CURSOR_OFF " "

#define CONF_GUI_KEYPAD_PASSWORD_LAST_CHAR_VISIBLE_DURATION 500000UL

namespace eez {
namespace psu {
namespace gui {
namespace keypad {

static const char *g_label;
int g_maxChars;

char g_keypadText[sizeof(Snapshot::text)];
static bool g_isUpperCase = false;

static bool g_isPassword = false;
static unsigned long g_lastKeyAppendTime;

void (*g_okCallback)(char *);
void (*g_cancelCallback)();

static bool g_cursor;
static unsigned long g_lastCursorChangeTime;

////////////////////////////////////////////////////////////////////////////////

void Snapshot::takeSnapshot(data::Snapshot *snapshot) {
	// text
	char *textPtr = this->text;

	if (g_label) {
		strcpy_P(textPtr, g_label);
		textPtr += strlen(g_label);
	}

	if (getActivePageId() == PAGE_ID_KEYPAD) {
		if (g_isPassword) {
			int n = strlen(keypad::g_keypadText);
			if (n > 0) {
				int i;

				for (i = 0; i < n - 1; ++i) {
					*textPtr++ = '*';
				}

				unsigned long current_time = micros();
				if (current_time - g_lastKeyAppendTime <= CONF_GUI_KEYPAD_PASSWORD_LAST_CHAR_VISIBLE_DURATION) {
					*textPtr++ = keypad::g_keypadText[i];
				} else {
					*textPtr++ = '*';
				}

				*textPtr++ = 0;
			} else {
				*textPtr = 0;
			}
		} else {
			strcpy(textPtr, keypad::g_keypadText);
		}

		appendCursor(this->text);
	} else {
		numeric_keypad::getText(textPtr, 16);
	}

	//
	if (getActivePageId() == PAGE_ID_KEYPAD) {
		this->isUpperCase = keypad::g_isUpperCase;
	}

	//
	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
        switch (numeric_keypad::getEditUnit()) {
        case data::VALUE_TYPE_FLOAT_VOLT: keypadUnit = data::Value::ProgmemStr(PSTR("mV")); break;
        case data::VALUE_TYPE_FLOAT_MILLI_VOLT: keypadUnit = data::Value::ProgmemStr(PSTR("V")); break;
        case data::VALUE_TYPE_FLOAT_AMPER: keypadUnit = data::Value::ProgmemStr(PSTR("mA")); break;
		case data::VALUE_TYPE_FLOAT_MILLI_AMPER: keypadUnit = data::Value::ProgmemStr(PSTR("A")); break;
        default: keypadUnit = data::Value::ProgmemStr(PSTR(""));
        }
	}
}

data::Value Snapshot::get(uint8_t id) {
	if (id == DATA_ID_KEYPAD_CAPS) {
		return isUpperCase ? 1 : 0;
	} else if (id == DATA_ID_EDIT_UNIT) {
        return keypadUnit;
    } else if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		data::Value value = numeric_keypad::getData(id);
		if (value.getType() != data::VALUE_TYPE_NONE) {
			return value;
		}
	}

	return data::Value();
}

////////////////////////////////////////////////////////////////////////////////

void init(const char *label_, void (*ok)(char *), void (*cancel)()) {
	g_label = label_;
	g_okCallback = ok;
	g_cancelCallback = cancel;

	g_lastCursorChangeTime = micros();
}

void start(const char *label, const char *text, int maxChars_, bool isPassword_, void (*ok)(char *), void (*cancel)()) {
	init(label, ok, cancel);

	g_maxChars = maxChars_;
	g_isPassword = isPassword_;

	if (text) {
		strcpy(g_keypadText, text);
	} else {
		g_keypadText[0] = 0;
	}
	g_isUpperCase = false;
}

void startPush(const char *label, const char *text, int maxChars_, bool isPassword_, void (*ok)(char *), void (*cancel)()) {
	start(label, text, maxChars_, isPassword_, ok, cancel);
	pushPage(PAGE_ID_KEYPAD);
}

void startReplace(const char *label, const char *text, int maxChars_, bool isPassword_, void (*ok)(char *), void (*cancel)()) {
	start(label, text, maxChars_, isPassword_, ok, cancel);
	replacePage(PAGE_ID_KEYPAD);
}

void appendChar(char c) {
	int n = strlen(g_keypadText);
	if (n < g_maxChars && (n + (g_label ? strlen(g_label) : 0)) < MAX_KEYPAD_TEXT_LENGTH) {
		g_keypadText[n] = c;
		g_keypadText[n + 1] = 0;
		g_lastKeyAppendTime = micros();
	} else {
		sound::playBeep();
	}
}

void key() {
	DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
	DECL_WIDGET_SPECIFIC(TextWidget, textWidget, widget);
	DECL_STRING(text, textWidget->text);
	char c = text[0];

	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		numeric_keypad::key(c);
	} else {
		appendChar(g_isUpperCase ? toupper(c) : tolower(c));
	}
}

void space() {
	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		numeric_keypad::space();
	} else {
		appendChar(' ');
	}
}

void caps() {
	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		numeric_keypad::caps();
	} else {
		g_isUpperCase = !g_isUpperCase;
	}
}

void back() {
	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		numeric_keypad::back();
	} else {
		int n = strlen(g_keypadText);
		if (n > 0) {
			g_keypadText[n - 1] = 0;
		} else {
			sound::playBeep();
		}
	}
}

void clear() {
	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		numeric_keypad::clear();
	} else {
		g_keypadText[0] = 0;
	}
}

void sign() {
	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		numeric_keypad::sign();
	} else {
	}
}

void unit() {
	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		numeric_keypad::unit();
	} else {
	}
}

void setMaxValue() {
	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		numeric_keypad::setMaxValue();
	} else {
	}
}

void setDefaultValue() {
	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		numeric_keypad::setDefaultValue();
	} else {
	}
}

void ok() {
	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		numeric_keypad::ok();
	} else {
		g_okCallback(g_keypadText);
	}
}

void cancel() {
	if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD || getActivePageId() == PAGE_ID_NUMERIC_KEYPAD) {
		numeric_keypad::cancel();
	} else {
		g_cancelCallback();
	}
}

void appendCursor(char *text) {
    unsigned long current_time = micros();
    if (current_time - g_lastCursorChangeTime > CONF_GUI_KEYPAD_CURSOR_BLINK_TIME) {
        g_cursor = !g_cursor;
        g_lastCursorChangeTime = current_time;
    }

    if (g_cursor) {
        strcat_P(text, PSTR(CONF_GUI_KEYPAD_CURSOR_ON));
    }
    else {
        strcat_P(text, PSTR(CONF_GUI_KEYPAD_CURSOR_OFF));
    }
}

}
}
}
} // namespace eez::psu::gui::keypad
