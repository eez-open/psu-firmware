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
#include "gui_keypad.h"

#include "sound.h"

#define CONF_KEYPAD_CURSOR_BLINK_TIME 500000UL
#define CONF_KEYPAD_CURSOR_ON "|"
#define CONF_KEYPAD_CURSOR_OFF " "

namespace eez {
namespace psu {
namespace gui {
namespace keypad {

static bool cursor;
static unsigned long last_cursor_change_time = 0;

static char old_value_text[32];
static char value_text[32];

static data::Unit edit_unit = data::UNIT_NONE;

enum State {
    START,
    EMPTY,
    D0,
    D1,
    DOT,
    D2,
    D3
};

static State state;

int d0;
int d1;
int d2;
int d3;

////////////////////////////////////////////////////////////////////////////////

bool isMilli() {
    return edit_unit == data::UNIT_MILLI_VOLT || edit_unit == data::UNIT_MILLI_AMPER;
}

float get_value() {
    float value = 0;

    if (state >= D0) {
        value = d0 * 1.0f;
    }

    if (state >= D1) {
        value = value * 10 + d1;
    }

    if (isMilli()) {
        if (state >= D2) {
            value = value * 10 + d2;
        }

        value /= 1000.0f;
    }
    else {
        value *= 100.0f;

        if (state >= D2) {
            value += d2 * 10;
        }

        if (state >= D3) {
            value += d3;
        }

        value /= 100.0f;
    }

    return value;
}

bool set_value(float fvalue) {
    if (isMilli()) {
        long value = (long)floor(fvalue * 1000);
        
        if (value > 999) {
            return false;
        }

        if (value >= 100) {
            d0 = value / 100;
            value = value % 100;
            d1 = value / 10;
            d2 = value % 10;
            state = D2;
        } else if (value >= 10) {
            d0 = value / 10;
            d1 = value % 10;
            state = D1;
        } else {
            d0  = value;
            state = D0;
        }
    } else {
        long value = (long)round(fvalue * 100);
        
        d3 = value % 10;
        if (d3 != 0) {
            state = D3;
            
            value /= 10;
            d2 = value % 10;
            
            value /= 10;
            d1 = value % 10;

            value /= 10;
            d0 = value % 10;

        } else {
            value /= 10;
            d2 = value % 10;
            if (d2 != 0) {
                state = D2;

                value /= 10;
                d1 = value % 10;

                value /= 10;
                d0 = value % 10;
            } else {
                value /= 10;
                d1 = value % 10;

                d0 = value / 10;
                if (d0 == 0) {
                    d0 = d1;
                    state = D0;
                } else {
                    state = D1;
                }
            }
        }
    }

    return true;
}

bool is_value_valid() {
    data::Cursor saved_cursor = data::getCursor();
    data::setCursor(edit_data_cursor);

    if (state == EMPTY) {
        return false;
    }

    float value = get_value();
    bool is_valid = value >= data::getMin(edit_data_id).getFloat() && value <= data::getMax(edit_data_id).getFloat();

    data::setCursor(saved_cursor);

    return is_valid;
}

void update_value_text() {
    if (state == START) {
        data::Cursor saved_cursor = data::getCursor();
        data::setCursor(edit_data_cursor);

        bool changed;
        data::get(edit_data_id, changed).toText(value_text, sizeof(value_text));

        data::setCursor(saved_cursor);
    }
    else {
        int i = 0;

        if (state >= D0 && (d0 != 0 || state < DOT)) {
            value_text[i++] = d0 + '0';
        }

        if (state >= D1) {
            value_text[i++] = d1 + '0';
        }

        if (!isMilli() && state >= DOT) {
            value_text[i++] = '.';
        }

        if (state >= D2) {
            value_text[i++] = d2 + '0';
        }

        if (state >= D3) {
            value_text[i++] = d3 + '0';
        }

        value_text[i] = 0;

        if (cursor) {
            strcat_P(value_text, PSTR(CONF_KEYPAD_CURSOR_ON));
        }
        else {
            strcat_P(value_text, PSTR(CONF_KEYPAD_CURSOR_OFF));
        }

        if (edit_unit == data::UNIT_VOLT)
            strcat_P(value_text, PSTR("V"));
        else if (edit_unit == data::UNIT_MILLI_VOLT)
            strcat_P(value_text, PSTR("mV"));
        else if (edit_unit == data::UNIT_AMPER)
            strcat_P(value_text, PSTR("A"));
        else if (edit_unit == data::UNIT_MILLI_AMPER)
            strcat_P(value_text, PSTR("mA"));
    }
}

void toggle_edit_unit() {
    if (edit_unit == data::UNIT_VOLT) {
        edit_unit = data::UNIT_MILLI_VOLT;
    }
    else if (edit_unit == data::UNIT_MILLI_VOLT) {
        edit_unit = data::UNIT_VOLT;
    }
    else if (edit_unit == data::UNIT_AMPER) {
        edit_unit = data::UNIT_MILLI_AMPER;
    }
    else if (edit_unit == data::UNIT_MILLI_AMPER) {
        edit_unit = data::UNIT_AMPER;
    }
}

////////////////////////////////////////////////////////////////////////////////

void reset() {
    state = START;
    edit_unit = data::getUnit(edit_data_id);
    update_value_text();
}

char *get_value_text(bool &changed) {
    changed = false;

    if (strcmp(old_value_text, value_text) != 0) {
        changed = true;
    }

    if (state != START) {
        unsigned long current_time = micros();
        if (current_time - last_cursor_change_time > CONF_KEYPAD_CURSOR_BLINK_TIME) {
            cursor = !cursor;
            last_cursor_change_time = current_time;
            update_value_text();
            changed = true;
        }
    }

    if (changed) {
        strcpy(old_value_text, value_text);
    }

    return old_value_text;
}

void do_action(int action_id) {
    if (action_id >= ACTION_ID_KEY_0 && action_id <= ACTION_ID_KEY_9) {
        int d = action_id - ACTION_ID_KEY_0;
        if (state == START || state == EMPTY) {
            d0 = d;
            state = D0;
            if (!is_value_valid()) {
                toggle_edit_unit();
            } 
            if (!is_value_valid()) {
                reset();
                sound::playBeep();
            }
        }
        else if (state == D0) {
            d1 = d;
            state = D1;
            if (!is_value_valid()) {
                state = D0;
                sound::playBeep();
            }
        }
        else if (state == DOT || isMilli() && state == D1) {
            State saved_state = state;
            d2 = d;
            state = D2;
            if (!is_value_valid()) {
                state = saved_state;
                sound::playBeep();
            }
        }
        else if (state == D2 && !isMilli()) {
            d3 = d;
            state = D3;
            if (!is_value_valid()) {
                state = D2;
                sound::playBeep();
            }
        }
        else {
            sound::playBeep();
        }
    }
    else if (action_id == ACTION_ID_KEY_DOT) {
        if (isMilli()) {
            sound::playBeep();
        } else {
            if (state == START || state == EMPTY) {
                d0 = 0;
                d1 = 0;
                state = DOT;
            }
            else if (state == D0) {
                d1 = d0;
                d0 = 0;
                state = DOT;
            }
            else if (state == D1) {
                state = DOT;
            }
            else {
                sound::playBeep();
            }
        }
    }
    else if (action_id == ACTION_ID_KEY_SIGN) {
        // not supported
        sound::playBeep();
    }
    else if (action_id == ACTION_ID_KEY_BACK) {
        if (state == D3) {
            state = D2;
        }
        else if (state == D2) {
            if (isMilli()) {
                state = D1;
            }
            else {
                state = DOT;
            }
        }
        else if (state == DOT) {
            if (d0 == 0) {
                d0 = d1;
                state = D0;
            }
            else {
                state = D1;
            }
        }
        else if (state == D1) {
            state = D0;
        }
        else if (state == D0) {
            state = EMPTY;
        }
        else {
            sound::playBeep();
        }
    }
    else if (action_id == ACTION_ID_KEY_C) {
        if (state != START) {
            reset();
        }
        else {
            sound::playBeep();
        }
    }
    else if (action_id == ACTION_ID_KEY_OK) {
        if (state != START && state != EMPTY) {
            data::Cursor saved_cursor = data::getCursor();
            data::setCursor(edit_data_cursor);
            data::set(edit_data_id, data::Value(get_value(), data::getUnit(edit_data_id)));
            data::setCursor(saved_cursor);

            reset();
        }
        else {
            sound::playBeep();
        }
    }
    else if (action_id == ACTION_ID_KEY_UNIT) {
        float value = get_value();
        data::Unit saved_edit_unit = edit_unit;

        toggle_edit_unit();

        if (state == START) {
            state = EMPTY;
        } else {
            if (state != EMPTY && !set_value(value)) {
                edit_unit = saved_edit_unit;
                sound::playBeep();
            }
        }
    }

    update_value_text();
}

data::Unit get_edit_unit() {
    return edit_unit;
}

}
}
}
} // namespace eez::psu::gui::keypad
