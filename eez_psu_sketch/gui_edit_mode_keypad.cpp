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
#include "gui_edit_mode.h"
#include "gui_edit_mode_keypad.h"

#include "sound.h"

#define CONF_KEYPAD_CURSOR_BLINK_TIME 500000UL
#define CONF_KEYPAD_CURSOR_ON "|"
#define CONF_KEYPAD_CURSOR_OFF " "

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode_keypad {

static bool cursor;
static unsigned long last_cursor_change_time = 0;

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

float getValue() {
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
    if (state == EMPTY) {
        return false;
    }
    float value = getValue();
    return (value >= edit_mode::getMin().getFloat() && value <= edit_mode::getMax().getFloat());
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
    edit_unit = edit_mode::getUnit();
}

void getText(char *text, int count) {
    if (state == START) {
        edit_mode::getCurrentValue().toText(text, count);
    }
    else {
        int i = 0;

        if (state >= D0 && (d0 != 0 || state < DOT)) {
            text[i++] = d0 + '0';
        }

        if (state >= D1) {
            text[i++] = d1 + '0';
        }

        if (!isMilli() && state >= DOT) {
            text[i++] = '.';
        }

        if (state >= D2) {
            text[i++] = d2 + '0';
        }

        if (state >= D3) {
            text[i++] = d3 + '0';
        }

        text[i] = 0;

        unsigned long current_time = micros();
        if (current_time - last_cursor_change_time > CONF_KEYPAD_CURSOR_BLINK_TIME) {
            cursor = !cursor;
            last_cursor_change_time = current_time;
        }

        if (cursor) {
            strcat_P(text, PSTR(CONF_KEYPAD_CURSOR_ON));
        }
        else {
            strcat_P(text, PSTR(CONF_KEYPAD_CURSOR_OFF));
        }

        if (edit_unit == data::UNIT_VOLT)
            strcat_P(text, PSTR("V"));
        else if (edit_unit == data::UNIT_MILLI_VOLT)
            strcat_P(text, PSTR("mV"));
        else if (edit_unit == data::UNIT_AMPER)
            strcat_P(text, PSTR("A"));
        else if (edit_unit == data::UNIT_MILLI_AMPER)
            strcat_P(text, PSTR("mA"));
    }
}

void doAction(int action_id) {
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
        else if (state == DOT || (isMilli() && state == D1)) {
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
            edit_mode::setValue(getValue());
            reset();
        }
        else {
            sound::playBeep();
        }
    }
    else if (action_id == ACTION_ID_KEY_UNIT) {
        float value = getValue();
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
}

data::Unit getEditUnit() {
    return edit_unit;
}

}
}
}
} // namespace eez::psu::gui::edit_mode_keypad
