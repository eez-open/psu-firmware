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
#include "datetime.h"
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

	switch (type_) {
	case VALUE_TYPE_NONE:
		break;

	case VALUE_TYPE_INT:
		util::strcatInt(text, int_);
		break;

	case VALUE_TYPE_CONST_STR:
		strncpy_P(text, const_str_, count - 1);
		text[count - 1] = 0;
		break;

	case VALUE_TYPE_STR:
		strncpy(text, str_, count - 1);
		text[count - 1] = 0;
		break;

	case VALUE_TYPE_CHANNEL_LABEL:
		snprintf_P(text, count-1, PSTR("Channel %d:"), int_);
		text[count - 1] = 0;
		break;

	case VALUE_TYPE_CHANNEL_SHORT_LABEL:
		snprintf_P(text, count-1, PSTR("Ch%d:"), int_);
		text[count - 1] = 0;
		break;

	case VALUE_TYPE_LESS_THEN_MIN:
		snprintf_P(text, count-1, PSTR("Value is less then %.2f"), float_);
		text[count - 1] = 0;
		break;

	case VALUE_TYPE_GREATER_THEN_MAX:
		snprintf_P(text, count-1, PSTR("Value is greater then %.2f"), float_);
		text[count - 1] = 0;
		break;

	case VALUE_TYPE_EVENT: 
		{
			int year, month, day, hour, minute, second;
			datetime::breakTime(event_->dateTime, year, month, day, hour, minute, second);

			int yearNow, monthNow, dayNow, hourNow, minuteNow, secondNow;
			datetime::breakTime(datetime::now(), yearNow, monthNow, dayNow, hourNow, minuteNow, secondNow);

			if (yearNow == year && monthNow == month && dayNow == day) {
				snprintf_P(text, count-1, PSTR("%c [%02d:%02d:%02d] %s"), 128 + event_->type, hour, minute, second, event_->message);
			} else {
				snprintf_P(text, count-1, PSTR("%c [%02d-%02d-%02d] %s"), 128 + event_->type, day, month, year % 1100, event_->message);
			}

			text[count - 1] = 0;
		}
		break;

	default:
		{
			util::strcatFloat(text, float_);

			const char *unit;

			switch (type_) {
			case VALUE_TYPE_FLOAT_VOLT:
				unit = "V";
				break;
			case VALUE_TYPE_FLOAT_AMPER:
				unit = "A";
				break;
			case VALUE_TYPE_FLOAT_WATT:
				unit = "W";
				break;
			case VALUE_TYPE_FLOAT_SECOND:
				unit = "s";
				break;
			case VALUE_TYPE_FLOAT_CELSIUS:
				unit = "oC";
				break;
			}

			if (unit) {
				strcat(text, unit);
			}
		}
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////

int count(uint8_t id) {
    if (id == DATA_ID_CHANNELS) {
        return CH_NUM;
    } else if (id == DATA_ID_EVENT_QUEUE_EVENTS) {
        return event_queue::getNumEvents();
    } 
    return 0;
}

void select(Cursor &cursor, uint8_t id, int index) {
    if (id == DATA_ID_CHANNELS) {
        cursor.iChannel = index;
    } else if (id == DATA_ID_EVENT_QUEUE_EVENTS) {
		cursor.iChannel = index;
	}
}

Value getMin(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_VOLT) {
        return Value(Channel::get(cursor.iChannel).U_MIN, VALUE_TYPE_FLOAT_VOLT);
    } else if (id == DATA_ID_CURR) {
        return Value(Channel::get(cursor.iChannel).I_MIN, VALUE_TYPE_FLOAT_AMPER);
    } else if (id == DATA_ID_EDIT_VALUE) {
        return edit_mode::getMin();
    }
    return Value();
}

Value getMax(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_VOLT) {
        return Value(Channel::get(cursor.iChannel).U_MAX, VALUE_TYPE_FLOAT_VOLT);
    } else if (id == DATA_ID_CURR) {
        return Value(Channel::get(cursor.iChannel).I_MAX, VALUE_TYPE_FLOAT_AMPER);
    } else if (id == DATA_ID_EDIT_VALUE) {
        return edit_mode::getMax();
    }
    return Value();
}

ValueType getUnit(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_VOLT) {
        return VALUE_TYPE_FLOAT_VOLT;
    } else if (id == DATA_ID_CURR) {
        return VALUE_TYPE_FLOAT_AMPER;
    }
    return VALUE_TYPE_NONE;
}

void getButtonLabels(const Cursor &cursor, uint8_t id, const Value **labels, int &count) {
    if (id == DATA_ID_EDIT_STEPS) {
        return edit_mode_step::getStepValues(labels, count);
    }
}

bool set(const Cursor &cursor, uint8_t id, Value value) {
    if (id == DATA_ID_VOLT) {
		if (value.getFloat() > Channel::get(cursor.iChannel).getVoltageLimit()) {
			return false;
		}
        if (value.getFloat() * Channel::get(cursor.iChannel).i.set > Channel::get(cursor.iChannel).getPowerLimit()) {
            return false;
        }
        Channel::get(cursor.iChannel).setVoltage(value.getFloat());
        return true;
    } else if (id == DATA_ID_CURR) {
		if (value.getFloat() > Channel::get(cursor.iChannel).getCurrentLimit()) {
			return false;
		}
        if (value.getFloat() * Channel::get(cursor.iChannel).u.set > Channel::get(cursor.iChannel).getPowerLimit()) {
            return false;
        }
        Channel::get(cursor.iChannel).setCurrent(value.getFloat());
        return true;
    } else if (id == DATA_ID_ALERT_MESSAGE) {
        g_alertMessage = value;
        return true;
    } else if (id == DATA_ID_EDIT_STEPS) {
        edit_mode_step::setStepIndex(value.getInt());
        return true;
    }
    return false;
}

}
}
}
} // namespace eez::psu::ui::data
