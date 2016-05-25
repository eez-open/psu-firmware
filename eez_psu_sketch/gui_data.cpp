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
#include "gui_internal.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_keypad.h"
#include "gui_edit_mode_step.h"
#include "channel.h"

namespace eez {
namespace psu {
namespace gui {
namespace data {

Value g_alertMessage;

////////////////////////////////////////////////////////////////////////////////

void Value::toText(char *text, int count) const {
    text[0] = 0;

    util::strcatFloat(text, float_);

    switch (unit_) {
    case UNIT_VOLT:
        strcat(text, " V");
        break;
    
    case UNIT_AMPER:
        strcat(text, " A");
        break;

    case UNIT_CONST_STR:
        strncpy_P(text, const_str_, count - 1);
        text[count - 1] = 0;
        break;

    case UNIT_STR:
        strncpy(text, str_, count - 1);
        text[count - 1] = 0;
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////

int count(uint8_t id) {
    if (id == DATA_ID_CHANNELS) {
        return CH_NUM;
    }
    return 0;
}

void select(Cursor &cursor, uint8_t id, int index) {
    if (id == DATA_ID_CHANNELS) {
        cursor.iChannel = index;
    }
}

Value getMin(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_VOLT) {
        return Value(Channel::get(cursor.iChannel).U_MIN, UNIT_VOLT);
    } else if (id == DATA_ID_CURR) {
        return Value(Channel::get(cursor.iChannel).I_MIN, UNIT_AMPER);
    } else if (id == DATA_ID_EDIT_VALUE) {
        return edit_mode::getMin();
    }
    return Value();
}

Value getMax(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_VOLT) {
        return Value(Channel::get(cursor.iChannel).U_MAX, UNIT_VOLT);
    } else if (id == DATA_ID_CURR) {
        return Value(Channel::get(cursor.iChannel).I_MAX, UNIT_AMPER);
    } else if (id == DATA_ID_EDIT_VALUE) {
        return edit_mode::getMax();
    }
    return Value();
}

Unit getUnit(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_VOLT) {
        return UNIT_VOLT;
    } else if (id == DATA_ID_CURR) {
        return UNIT_AMPER;
    }
    return UNIT_NONE;
}

void getButtonLabels(const Cursor &cursor, uint8_t id, const Value **labels, int &count) {
    if (id == DATA_ID_EDIT_STEPS) {
        return edit_mode_step::getStepValues(labels, count);
    }
}

void set(const Cursor &cursor, uint8_t id, Value value) {
    if (id == DATA_ID_VOLT) {
        Channel::get(cursor.iChannel).setVoltage(value.getFloat());
    } else if (id == DATA_ID_CURR) {
        Channel::get(cursor.iChannel).setCurrent(value.getFloat());
    } else if (id == DATA_ID_ALERT_MESSAGE) {
        g_alertMessage = value;
    } else if (id == DATA_ID_EDIT_STEPS) {
        edit_mode_step::setStepIndex(value.getInt());
    }
}

void toggle(uint8_t id) {
    if (id == DATA_ID_EDIT_MODE_INTERACTIVE_MODE_SELECTOR) {
        edit_mode::toggleInteractiveMode();
    }
}

void doAction(const Cursor &cursor, uint8_t id) {
    if (id == ACTION_ID_TOGGLE_CHANNEL) {
        Channel::get(cursor.iChannel).outputEnable(!Channel::get(cursor.iChannel).isOutputEnabled());
    }
}

}
}
}
} // namespace eez::psu::ui::data
