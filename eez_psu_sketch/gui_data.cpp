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

#include "datetime.h"
#include "profile.h"
#include "channel.h"
#include "channel_dispatcher.h"
#include "calibration.h"
#include "trigger.h"
#include "list.h"

#include "gui.h"
#include "gui_internal.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_keypad.h"
#include "gui_edit_mode_step.h"
#include "gui_calibration.h"
#include "gui_keypad.h"
#include "gui_page_ch_settings_trigger.h"
#include "serial_psu.h"
#include "ethernet.h"

#define CONF_GUI_REFRESH_EVERY_MS 250

namespace eez {
namespace psu {
namespace gui {
namespace data {

Value g_alertMessage;
Value g_alertMessage2;
Value g_alertMessage3;

static struct ChannelSnapshot {
    unsigned int mode;
    Value monValue;
    float pMon;
    uint32_t lastSnapshotTime;
} g_channelSnapshot[CH_MAX];

////////////////////////////////////////////////////////////////////////////////

data::EnumItem g_channelDisplayValueEnumDefinition[] = {
    {DISPLAY_VALUE_VOLTAGE, PSTR("Voltage (V)")},
    {DISPLAY_VALUE_CURRENT, PSTR("Current (A)")},
    {DISPLAY_VALUE_POWER, PSTR("Power (W)")},
    {0, 0}
};

data::EnumItem g_channelTriggerModeEnumDefinition[] = {
    {TRIGGER_MODE_FIXED, PSTR("Fixed")},
    {TRIGGER_MODE_LIST, PSTR("List")},
    {TRIGGER_MODE_STEP, PSTR("Step")},
    {0, 0}
};

data::EnumItem g_triggerSourceEnumDefinition[] = {
    {trigger::SOURCE_BUS, PSTR("Bus")},
    {trigger::SOURCE_IMMEDIATE, PSTR("Immediate")},
    {trigger::SOURCE_MANUAL, PSTR("Manual")},
    {trigger::SOURCE_PIN1, PSTR("Pin1")},
    {0, 0}
};

data::EnumItem g_triggerPolarityEnumDefinition[] = {
    {trigger::POLARITY_NEGATIVE, PSTR("Negative")},
    {trigger::POLARITY_POSITIVE, PSTR("Positive")},
    {0, 0}
};

data::EnumItem g_channelCurrentRangeSelectionMode[] = {
    {CURRENT_RANGE_SELECTION_USE_BOTH, PSTR("Best (default)")},
    {CURRENT_RANGE_SELECTION_ALWAYS_HIGH, PSTR("5A")},
    {CURRENT_RANGE_SELECTION_ALWAYS_LOW, PSTR("0.5A")},
    {0, 0}
};

data::EnumItem g_channelCurrentRange[] = {
    {CURRENT_RANGE_HIGH, PSTR("5A")},
    {CURRENT_RANGE_LOW, PSTR("0.5A")},
    {0, 0}
};

data::EnumItem g_channelTriggerOnListStop[] = {
    {TRIGGER_ON_LIST_STOP_OUTPUT_OFF, PSTR("Output OFF")},
    {TRIGGER_ON_LIST_STOP_SET_TO_FIRST_STEP, PSTR("Set to first step")},
    {TRIGGER_ON_LIST_STOP_SET_TO_LAST_STEP, PSTR("Set to last step")},
    {TRIGGER_ON_LIST_STOP_STANDBY, PSTR("Standby")},
    {0, 0}
};

static const data::EnumItem *enumDefinitions[] = {
    g_channelDisplayValueEnumDefinition,
    g_channelTriggerModeEnumDefinition,
    g_triggerSourceEnumDefinition,
    g_triggerPolarityEnumDefinition,
    g_channelCurrentRangeSelectionMode,
    g_channelCurrentRange,
    g_channelTriggerOnListStop
};

////////////////////////////////////////////////////////////////////////////////

Value::Value(float value, ValueType type)
    : type_(type), float_(value)
{
    options_ = 0;
}


Value::Value(float value, ValueType type, int channelIndex) 
    : type_(type), float_(value)
{
    options_ = channelIndex;
}

Value::Value(float value, ValueType type, int channelIndex, bool extendedPrecision) 
    : type_(type), float_(value)
{
    options_ = channelIndex;
    if (extendedPrecision) {
        options_ |= VALUE_OPTIONS_EXTENDED_PRECISION;
    }
}

Value Value::ProgmemStr(const char *pstr PROGMEM) {
    Value value;
    value.const_str_ = pstr;
    value.type_ = VALUE_TYPE_CONST_STR;
    return value;
}

Value Value::PageInfo(uint8_t pageIndex, uint8_t numPages) {
	Value value;
	value.pageInfo_.pageIndex = pageIndex;
	value.pageInfo_.numPages = numPages;
	value.type_ = VALUE_TYPE_PAGE_INFO;
	return value;
}

Value Value::ScpiErrorText(int16_t errorCode) {
	Value value;
	value.int16_ = errorCode;
	value.type_ = VALUE_TYPE_SCPI_ERROR_TEXT;
	return value;
}

Value Value::LessThenMinMessage(float float_, ValueType type) {
	Value value;
	if (type == VALUE_TYPE_INT) {
		value.int_ = int(float_);
		value.type_ = VALUE_TYPE_LESS_THEN_MIN_INT;
	} else if (type == VALUE_TYPE_TIME_ZONE) {
		value.type_ = VALUE_TYPE_LESS_THEN_MIN_TIME_ZONE;
	} else {
		value.float_ = float_;
		value.type_ = VALUE_TYPE_LESS_THEN_MIN_FLOAT + type - VALUE_TYPE_FLOAT;
	}
	return value;
}

Value Value::GreaterThenMaxMessage(float float_, ValueType type) {
	Value value;
	if (type == VALUE_TYPE_INT) {
		value.int_ = int(float_);
		value.type_ = VALUE_TYPE_GREATER_THEN_MAX_INT;
	} else if (type == VALUE_TYPE_TIME_ZONE) {
		value.type_ = VALUE_TYPE_GREATER_THEN_MAX_TIME_ZONE;
	} else {
		value.float_ = float_;
		value.type_ = VALUE_TYPE_GREATER_THEN_MAX_FLOAT + type - VALUE_TYPE_FLOAT;
	}
	return value;
}

bool Value::isMilli() const {
    if (type_ == VALUE_TYPE_FLOAT_VOLT || type_ == VALUE_TYPE_FLOAT_AMPER || type_ == VALUE_TYPE_FLOAT_WATT || type_ == VALUE_TYPE_FLOAT_SECOND) {
        int numSignificantDecimalDigits = getNumSignificantDecimalDigits((ValueType)type_);
        if (options_ & VALUE_OPTIONS_EXTENDED_PRECISION) {
            ++numSignificantDecimalDigits;
        }
        float precision = getPrecisionFromNumSignificantDecimalDigits(numSignificantDecimalDigits);
        return util::greater(float_, -1.0f, precision) && util::less(float_, 1.0f, precision) && !util::equal(float_, 0, precision);
    }
    return false;
}

void Value::formatFloatValue(float &value, ValueType &valueType, int &numSignificantDecimalDigits) const {
    value = float_;
    valueType = (ValueType)type_;
    numSignificantDecimalDigits = getNumSignificantDecimalDigits((ValueType)type_);

    bool forceNumSignificantDecimalDigits = false;

    if (options_ & VALUE_OPTIONS_EXTENDED_PRECISION) {
        ++numSignificantDecimalDigits;
        forceNumSignificantDecimalDigits = true;
    }

    if (isMilli()) {
        value *= 1000.0f;

        if (valueType == VALUE_TYPE_FLOAT_VOLT) {
            valueType = VALUE_TYPE_FLOAT_MILLI_VOLT;
            if (!forceNumSignificantDecimalDigits) {
                if (numSignificantDecimalDigits == 2) {
                    value = roundf(value / 10) * 10;
                }
                numSignificantDecimalDigits = 0;
            }
        } else if (valueType == VALUE_TYPE_FLOAT_AMPER) {
            valueType = VALUE_TYPE_FLOAT_MILLI_AMPER;
            if (!forceNumSignificantDecimalDigits) {
                if (channel_dispatcher::isCurrentLowRangeAllowed(Channel::get(options_ & VALUE_OPTIONS_CH_MASK)) && util::lessOrEqual(value, 0.5, getPrecision(VALUE_TYPE_FLOAT_AMPER))) {
                    numSignificantDecimalDigits = 1;
                } else {
                    numSignificantDecimalDigits = 0;
                }
            }
        } else if (valueType == VALUE_TYPE_FLOAT_WATT) {
            valueType = VALUE_TYPE_FLOAT_MILLI_WATT;
            if (!forceNumSignificantDecimalDigits) {
                numSignificantDecimalDigits = 0;
            }
        } else if (valueType == VALUE_TYPE_FLOAT_SECOND) {
            valueType = VALUE_TYPE_FLOAT_MILLI_SECOND;
            if (!forceNumSignificantDecimalDigits) {
                numSignificantDecimalDigits = 1;
            }
        }

        if (forceNumSignificantDecimalDigits) {
            numSignificantDecimalDigits -= 3;
        }
    }

    if (!forceNumSignificantDecimalDigits) {
        if (numSignificantDecimalDigits > 3) {
            numSignificantDecimalDigits = 3;
        }

        if (numSignificantDecimalDigits > 2 && util::greater(value, 9.999f, getPrecisionFromNumSignificantDecimalDigits(3))) {
            numSignificantDecimalDigits = 2;
        }

        if (numSignificantDecimalDigits > 1 && util::greater(value, 99.99f, getPrecisionFromNumSignificantDecimalDigits(2))) {
            numSignificantDecimalDigits = 1;
        }
    }
}

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
        snprintf_P(text, count-1, PSTR("Channel %d:"), uint8_);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_CHANNEL_SHORT_LABEL:
        snprintf_P(text, count-1, PSTR("Ch%d:"), uint8_);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_CHANNEL_BOARD_INFO_LABEL:
        snprintf_P(text, count-1, PSTR("CH%d board:"), int_);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_LESS_THEN_MIN_INT:
        snprintf_P(text, count-1, PSTR("Value is less then %d"), int_);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_LESS_THEN_MIN_TIME_ZONE:
        strncpy_P(text, PSTR("Value is less then -12:00"), count-1);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_GREATER_THEN_MAX_INT:
        snprintf_P(text, count-1, PSTR("Value is greater then %d"), int_);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_GREATER_THEN_MAX_TIME_ZONE:
        strncpy_P(text, PSTR("Value is greater then +14:00"), count-1);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_EVENT: 
        {
            int year, month, day, hour, minute, second;
            datetime::breakTime(event_->dateTime, year, month, day, hour, minute, second);

            int yearNow, monthNow, dayNow, hourNow, minuteNow, secondNow;
            datetime::breakTime(datetime::now(), yearNow, monthNow, dayNow, hourNow, minuteNow, secondNow);

            if (yearNow == year && monthNow == month && dayNow == day) {
                snprintf_P(text, count-1, PSTR("%c [%02d:%02d:%02d] %s"), 127 + event_queue::getEventType(event_), hour, minute, second, event_queue::getEventMessage(event_));
            } else {
                snprintf_P(text, count-1, PSTR("%c [%02d-%02d-%02d] %s"), 127 + event_queue::getEventType(event_), day, month, year % 100, event_queue::getEventMessage(event_));
            }

            text[count - 1] = 0;
        }
        break;

    case VALUE_TYPE_PAGE_INFO:
        snprintf_P(text, count-1, PSTR("Page #%d of %d"), pageInfo_.pageIndex + 1, pageInfo_.numPages);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_ON_TIME_COUNTER:
        ontime::counterToString(text, count, uint32_);
        break;

    case VALUE_TYPE_COUNTDOWN:
    {
        int h = uint32_ / 3600;
        int r = uint32_ - h * 3600;
        int m = r / 60;
        int s = r - m * 60;
        snprintf_P(text, count-1, PSTR("%02d:%02d:%02d"), h, m, s);
	    text[count-1] = 0;
        break;
    }

    case VALUE_TYPE_SCPI_ERROR_TEXT:
        strncpy(text, SCPI_ErrorTranslate(int16_), count - 1);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_TIME_ZONE:
        if (int16_ == 0) {
            strncpy_P(text, PSTR("GMT"), count - 1);
            text[count - 1] = 0;
        } else {
            char sign;
            int16_t value;
            if (int16_ > 0) {
                sign = '+';
                value = int16_;
            } else {
                sign = '-';
                value = -int16_;
            }
            snprintf_P(text, count-1, PSTR("%c%02d:%02d GMT"), sign, value / 100, value % 100);
            text[count - 1] = 0;
        }
        break;

    case VALUE_TYPE_YEAR:
        snprintf_P(text, count-1, PSTR("%d"), uint16_);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_MONTH:
    case VALUE_TYPE_DAY:
    case VALUE_TYPE_HOUR:
    case VALUE_TYPE_MINUTE:
    case VALUE_TYPE_SECOND:
        snprintf_P(text, count-1, PSTR("%02d"), uint8_);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_USER_PROFILE_LABEL:
        snprintf_P(text, count-1, PSTR("[ %d ]"), int_);
        text[count - 1] = 0;
        break;

    case VALUE_TYPE_USER_PROFILE_REMARK:
        profile::getName(int_, text, count);
        break;

    case VALUE_TYPE_EDIT_INFO:
        edit_mode::getInfoText(int_, text);
        break;

    case VALUE_TYPE_IP_ADDRESS:
    {
        uint8_t *bytes = (uint8_t *)&uint32_;
        snprintf_P(text, count-1, PSTR("%d.%d.%d.%d"), (int)bytes[0], (int)bytes[1], (int)bytes[2], (int)bytes[3]);
        text[count - 1] = 0;
        break;
    }

    case VALUE_TYPE_ENUM:
    {
        const EnumItem *enumDefinition = enumDefinitions[enum_.enumDefinition];
        for (int i = 0; enumDefinition[i].label; ++i) {
            if (enum_.value == enumDefinition[i].value) {
                strncpy_P(text, enumDefinition[i].label, count-1);
                break;
            }
        }
        break;
    }

    case VALUE_TYPE_TEXT_MESSAGE:
        strncpy(text, getTextMessage(), count - 1);
        text[count - 1] = 0;
        break;

    default:
        if (type_ > VALUE_TYPE_GREATER_THEN_MAX_FLOAT) {
            char valueText[64];
            Value(float_, ValueType(type_ - VALUE_TYPE_GREATER_THEN_MAX_FLOAT + VALUE_TYPE_FLOAT)).toText(valueText, sizeof(text));
            snprintf_P(text, count-1, PSTR("Value is greater then %s"), valueText);
            text[count - 1] = 0;
        } else if (type_ > VALUE_TYPE_LESS_THEN_MIN_FLOAT) {
            char valueText[64];
            Value(float_, ValueType(type_ - VALUE_TYPE_LESS_THEN_MIN_FLOAT + VALUE_TYPE_FLOAT)).toText(valueText, sizeof(text));
            snprintf_P(text, count-1, PSTR("Value is less then %s"), valueText);
            text[count - 1] = 0;
        } else {
            float value;
            ValueType valueType;
            int numSignificantDecimalDigits;

            formatFloatValue(value, valueType, numSignificantDecimalDigits);

            text[0] = 0;
            util::strcatFloat(text, value, numSignificantDecimalDigits);
            if (type_ == VALUE_TYPE_FLOAT_SECOND) {
                util::removeTrailingZerosFromFloat(text);
            }
            strcat(text, getUnitStr(valueType));
        }
        break;
    }
}

bool Value::operator ==(const Value &other) const {
    if (type_ != other.type_) {
        return false;
    }

    if (type_ == VALUE_TYPE_NONE || type_ == VALUE_TYPE_LESS_THEN_MIN_TIME_ZONE || type_ == VALUE_TYPE_GREATER_THEN_MAX_TIME_ZONE) {
        return true;
    }
		
	if (type_ == VALUE_TYPE_INT || type_ == VALUE_TYPE_CHANNEL_BOARD_INFO_LABEL || type_ == VALUE_TYPE_LESS_THEN_MIN_INT || type_ == VALUE_TYPE_GREATER_THEN_MAX_INT || type_ == VALUE_TYPE_USER_PROFILE_LABEL || type_ == VALUE_TYPE_USER_PROFILE_REMARK || type_ == VALUE_TYPE_EDIT_INFO) {
        return int_ == other.int_;
    }

	if (type_ == VALUE_TYPE_SCPI_ERROR_TEXT || type_ == VALUE_TYPE_TIME_ZONE) {
		return int16_ == other.int16_;
	}

	if (type_ >= VALUE_TYPE_MONTH && type_ <= VALUE_TYPE_SECOND || type_ == VALUE_TYPE_TEXT_MESSAGE || type_ == VALUE_TYPE_CHANNEL_LABEL || type_ == VALUE_TYPE_CHANNEL_SHORT_LABEL) {
		return uint8_ == other.uint8_;
	}

	if (type_ == VALUE_TYPE_YEAR) {
		return uint16_ == other.uint16_;
	}

	if (type_ == VALUE_TYPE_ON_TIME_COUNTER || type_ == VALUE_TYPE_COUNTDOWN || type_ == VALUE_TYPE_IP_ADDRESS) {
		return uint32_ == other.uint32_;
	}
		
	if (type_ == VALUE_TYPE_STR) {
        return strcmp(str_, other.str_) == 0;
    }

	if (type_ == VALUE_TYPE_CONST_STR) {
        return const_str_ == other.const_str_;
    }
		
	if (type_ == VALUE_TYPE_EVENT) {
        return event_->dateTime == other.event_->dateTime && event_->eventId == other.event_->eventId;
    }
		
	if (type_ == VALUE_TYPE_PAGE_INFO) {
		return pageInfo_.pageIndex == other.pageInfo_.pageIndex && pageInfo_.numPages == other.pageInfo_.numPages;
	}

    if (type_ == VALUE_TYPE_ENUM) {
        return enum_.enumDefinition == other.enum_.enumDefinition && enum_.value == other.enum_.value;
    }
    
    if (type_ == VALUE_TYPE_FLOAT_SECOND) {
    	return util::equal(float_, other.float_, powf(10.0f, 4));
    }

    if (float_ == other.float_) {
        return true;
    }

    float value;
    ValueType valueType;
    int numSignificantDecimalDigits;
    formatFloatValue(value, valueType, numSignificantDecimalDigits);

    float otherValue;
    ValueType otherValueType;
    int otherNumSignificantDecimalDigits;
    other.formatFloatValue(otherValue, otherValueType, otherNumSignificantDecimalDigits);

    if (valueType != otherValueType) {
        return false;
    }

    if (numSignificantDecimalDigits != otherNumSignificantDecimalDigits) {
        return false;
    }

    return util::equal(value, otherValue, getPrecisionFromNumSignificantDecimalDigits(numSignificantDecimalDigits));
}

int Value::getInt() const {
    if (type_ == VALUE_TYPE_ENUM) {
        return enum_.value;
    }
    return int_; 
}

////////////////////////////////////////////////////////////////////////////////

static bool isDisplayValue(const Cursor &cursor, uint8_t id, DisplayValue displayValue) {
    return cursor.i >= 0 && 
        (id == DATA_ID_CHANNEL_DISPLAY_VALUE1 && Channel::get(cursor.i).flags.displayValue1 == displayValue ||
         id == DATA_ID_CHANNEL_DISPLAY_VALUE2 && Channel::get(cursor.i).flags.displayValue2 == displayValue);
}

static bool isUMonData(const Cursor &cursor, uint8_t id) {
    return id == DATA_ID_CHANNEL_U_MON || isDisplayValue(cursor, id, DISPLAY_VALUE_VOLTAGE);
}

static bool isIMonData(const Cursor &cursor, uint8_t id) {
    return id == DATA_ID_CHANNEL_I_MON || isDisplayValue(cursor, id, DISPLAY_VALUE_CURRENT);
}

static bool isPMonData(const Cursor &cursor, uint8_t id) {
    return id == DATA_ID_CHANNEL_P_MON || isDisplayValue(cursor, id, DISPLAY_VALUE_POWER);
}

int count(uint8_t id) {
    if (id == DATA_ID_CHANNELS) {
        return CH_MAX;
    } else if (id == DATA_ID_EVENT_QUEUE_EVENTS) {
        return event_queue::EVENTS_PER_PAGE;
    } else if (id == DATA_ID_PROFILES_LIST1) {
        return 4;
    } else if (id == DATA_ID_PROFILES_LIST2) {
        return 6;
    } else if (id == DATA_ID_CHANNEL_LISTS) {
        return LIST_ITEMS_PER_PAGE;
    }
    return 0;
}

void select(Cursor &cursor, uint8_t id, int index) {
    if (id == DATA_ID_CHANNELS) {
        cursor.i = index;
    } else if (id == DATA_ID_EVENT_QUEUE_EVENTS) {
        cursor.i = index;
    } else if (id == DATA_ID_PROFILES_LIST1) {
        cursor.i = index;
    } else if (id == DATA_ID_PROFILES_LIST2) {
        cursor.i = 4 + index;
    } else if (id == DATA_ID_CHANNEL_COUPLING_MODE) {
        cursor.i = 0;
    } else if (id == DATA_ID_CHANNEL_LISTS) {
        cursor.i = index;
    }
}

int getListLength(uint8_t id) {
    Page *activePage = getActivePage();
    if (activePage) {
        return activePage->getListLength(id);
    }

    return 0;
}

float *getFloatList(uint8_t id) {
    Page *activePage = getActivePage();
    if (activePage) {
        return activePage->getFloatList(id);
    }

    return 0;
}

Value getMin(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_CHANNEL_U_SET || id == DATA_ID_CHANNEL_U_EDIT || isUMonData(cursor, id)) {
        return Value(channel_dispatcher::getUMin(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_VOLT, cursor.i);
    } else if (id == DATA_ID_CHANNEL_I_SET || id == DATA_ID_CHANNEL_I_EDIT || isIMonData(cursor, id)) {
        return Value(channel_dispatcher::getIMin(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_AMPER, cursor.i);
    } else if (isPMonData(cursor, id)) {
        return Value(channel_dispatcher::getPowerMinLimit(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_WATT, cursor.i);
    } else if (id == DATA_ID_EDIT_VALUE) {
        return edit_mode::getMin();
    }

    Page *activePage = getActivePage();
    if (activePage) {
        Value value = activePage->getMin(cursor, id);
        if (value.getType() != VALUE_TYPE_NONE) {
            return value;
        }
    }

    return Value();
}

Value getMax(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_CHANNEL_U_SET || id == DATA_ID_CHANNEL_U_EDIT || isUMonData(cursor, id)) {
        return Value(channel_dispatcher::getUMax(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_VOLT, cursor.i);
    } else if (id == DATA_ID_CHANNEL_I_SET || id == DATA_ID_CHANNEL_I_EDIT || isIMonData(cursor, id)) {
        return Value(channel_dispatcher::getIMax(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_AMPER, cursor.i);
    } else if (isPMonData(cursor, id)) {
        return Value(channel_dispatcher::getPowerMaxLimit(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_WATT, cursor.i);
    } else if (id == DATA_ID_EDIT_VALUE) {
        return edit_mode::getMax();
    }

    Page *activePage = getActivePage();
    if (activePage) {
        Value value = activePage->getMax(cursor, id);
        if (value.getType() != VALUE_TYPE_NONE) {
            return value;
        }
    }

    return Value();
}

Value getDef(const Cursor &cursor, uint8_t id) {
    Page *activePage = getActivePage();
    if (activePage) {
        Value value = activePage->getDef(cursor, id);
        if (value.getType() != VALUE_TYPE_NONE) {
            return value;
        }
    }

    return Value();
}

Value getLimit(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_CHANNEL_U_SET || id == DATA_ID_CHANNEL_U_EDIT || isUMonData(cursor, id)) {
        return Value(channel_dispatcher::getULimit(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_VOLT, cursor.i);
    } else if (id == DATA_ID_CHANNEL_I_SET || id == DATA_ID_CHANNEL_I_EDIT || isIMonData(cursor, id)) {
        return Value(channel_dispatcher::getILimit(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_AMPER, cursor.i);
    } else if (isPMonData(cursor, id)) {
        return Value(channel_dispatcher::getPowerLimit(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_WATT, cursor.i);
    }

    return Value();
}

ValueType getUnit(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_CHANNEL_U_SET || id == DATA_ID_CHANNEL_U_EDIT) {
        return VALUE_TYPE_FLOAT_VOLT;
    } else if (id == DATA_ID_CHANNEL_I_SET || id == DATA_ID_CHANNEL_I_EDIT) {
        return VALUE_TYPE_FLOAT_AMPER;
    }
    return VALUE_TYPE_NONE;
}

void getList(const Cursor &cursor, uint8_t id, const Value **values, int &count) {
    if (id == DATA_ID_EDIT_STEPS) {
        return edit_mode_step::getStepValues(values, count);
    }
}

/*
We are auto generating model name from the channels definition:

<cnt>/<volt>/<curr>[-<cnt2>/<volt2>/<curr2>] (<platform>)

Where is:

<cnt>      - number of the equivalent channels
<volt>     - max. voltage
<curr>     - max. curr
<platform> - Mega, Due, Simulator or Unknown
*/
const char *getModelInfo() {
    static char model_info[CH_NUM * (sizeof("XX V / XX A") - 1) + CH_NUM * (sizeof(" - ") - 1) + 1];

    if (*model_info == 0) {
        char *p = Channel::getChannelsInfoShort(model_info);
        *p = 0;
    }

    return model_info;
}

const char *getFirmwareInfo() {
    static const char FIRMWARE_LABEL[] PROGMEM = "Firmware: ";
    static char firmware_info[sizeof(FIRMWARE_LABEL) - 1 + sizeof(FIRMWARE) - 1 + 1];

    if (*firmware_info == 0) {
        strcat_P(firmware_info, FIRMWARE_LABEL);
        strcat_P(firmware_info, PSTR(FIRMWARE));
    }

    return firmware_info;
}

Value get(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_CHANNELS_VIEW_MODE) {
        return Value(persist_conf::devConf.flags.channelsViewMode);
    }

    if (id == DATA_ID_CHANNEL_COUPLING_MODE) {
        return data::Value(channel_dispatcher::getType());
    }

    if (id == DATA_ID_CHANNEL_IS_COUPLED) {
        return data::Value(channel_dispatcher::isCoupled() ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_IS_TRACKED) {
        return data::Value(channel_dispatcher::isTracked() ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_IS_COUPLED_OR_TRACKED) {
        return data::Value(channel_dispatcher::isCoupled() || channel_dispatcher::isTracked() ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_COUPLING_IS_SERIES) {
        return data::Value(channel_dispatcher::isSeries() ? 1 : 0);
    }

    int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : -1);
    
    if (iChannel >= 0) {
        Channel &channel = Channel::get(iChannel);

        int channelStatus = channel.index > CH_NUM ? 0 : (channel.isOk() ? 1 : 2);

        if (id == DATA_ID_CHANNEL_STATUS) {
            return Value(channelStatus);
        }

        if (channelStatus == 1) {
            if (id == DATA_ID_CHANNEL_OUTPUT_STATE) {
                return Value(channel.isOutputEnabled() ? 1 : 0);
            }

            ChannelSnapshot &channelSnapshot = g_channelSnapshot[channel.index - 1];
            uint32_t currentTime = micros();
            if (!channelSnapshot.lastSnapshotTime || currentTime - channelSnapshot.lastSnapshotTime >= CONF_GUI_REFRESH_EVERY_MS * 1000UL) {
                char *mode_str = channel.getCvModeStr();
                channelSnapshot.mode = 0;
                float uMon = channel_dispatcher::getUMon(channel);
                float iMon = channel_dispatcher::getIMon(channel);
                if (strcmp(mode_str, "CC") == 0) {
                    channelSnapshot.monValue = Value(uMon, VALUE_TYPE_FLOAT_VOLT, channel.index-1);
                } else if (strcmp(mode_str, "CV") == 0) {
                    channelSnapshot.monValue = Value(iMon, VALUE_TYPE_FLOAT_AMPER, channel.index-1);
                } else {
                    channelSnapshot.mode = 1;
                    if (uMon < iMon) {
                        channelSnapshot.monValue = Value(uMon, VALUE_TYPE_FLOAT_VOLT, channel.index-1);
                    } else {
                        channelSnapshot.monValue = Value(iMon, VALUE_TYPE_FLOAT_AMPER, channel.index-1);
                    }
                }

                channelSnapshot.pMon = util::multiply(uMon, iMon, getPrecision(VALUE_TYPE_FLOAT_WATT));

                channelSnapshot.lastSnapshotTime = currentTime;
            }

            if (id == DATA_ID_CHANNEL_OUTPUT_MODE) {
                char *mode_str = channel.getCvModeStr();
                return Value(channelSnapshot.mode);
            }

            if (id == DATA_ID_EDIT_ENABLED) {
                if ((channel_dispatcher::getVoltageTriggerMode(channel) != TRIGGER_MODE_FIXED && !trigger::isIdle()) || isPageActiveOrOnStack(PAGE_ID_CH_SETTINGS_LISTS)) {
                    return 0;
                }
                if (psu::calibration::isEnabled()) {
                    return 0;
                }
                return 1;
            }

            if (id == DATA_ID_TRIGGER_IS_INITIATED) {
                return Value(trigger::isInitiated() ? 1 : 0);
            }
        
            if (id == DATA_ID_TRIGGER_IS_MANUAL) {
                return Value(trigger::getSource() == trigger::SOURCE_MANUAL ? 1 : 0);
            }

            if (id == DATA_ID_CHANNEL_MON_VALUE) {
                return channelSnapshot.monValue;
            }
        
            if (id == DATA_ID_CHANNEL_U_SET) {
                return Value(channel_dispatcher::getUSet(channel), VALUE_TYPE_FLOAT_VOLT, channel.index-1);
            }
        
            if (id == DATA_ID_CHANNEL_U_EDIT) {
                if (g_focusCursor == cursor && g_focusDataId == DATA_ID_CHANNEL_U_EDIT && g_focusEditValue.getType() != VALUE_TYPE_NONE) {
                    return g_focusEditValue;
                } else {
                    return Value(channel_dispatcher::getUSet(channel), VALUE_TYPE_FLOAT_VOLT, channel.index-1);
                }
            }
        
            if (isUMonData(cursor, id)) {
                return Value(channel_dispatcher::getUMon(channel), VALUE_TYPE_FLOAT_VOLT, channel.index-1);
            }

            if (id == DATA_ID_CHANNEL_U_MON_DAC) {
                return Value(channel_dispatcher::getUMonDac(channel), VALUE_TYPE_FLOAT_VOLT, channel.index-1);
            }

            if (id == DATA_ID_CHANNEL_U_LIMIT) {
                return Value(channel_dispatcher::getULimit(channel), VALUE_TYPE_FLOAT_VOLT, channel.index-1);
            }

            if (id == DATA_ID_CHANNEL_I_SET) {
                return Value(channel_dispatcher::getISet(channel), VALUE_TYPE_FLOAT_AMPER, channel.index-1);
            }
        
            if (id == DATA_ID_CHANNEL_I_EDIT) {
                if (g_focusCursor == cursor && g_focusDataId == DATA_ID_CHANNEL_I_EDIT && g_focusEditValue.getType() != VALUE_TYPE_NONE) {
                    return g_focusEditValue;
                } else {
                    return Value(channel_dispatcher::getISet(channel), VALUE_TYPE_FLOAT_AMPER, channel.index-1);
                }
            }

            if (isIMonData(cursor, id)) {
                return Value(channel_dispatcher::getIMon(channel), VALUE_TYPE_FLOAT_AMPER, channel.index-1);
            }

            if (id == DATA_ID_CHANNEL_I_MON_DAC) {
                return Value(channel_dispatcher::getIMonDac(channel), VALUE_TYPE_FLOAT_AMPER, channel.index-1);
            }

            if (id == DATA_ID_CHANNEL_I_LIMIT) {
                return Value(channel_dispatcher::getILimit(channel), VALUE_TYPE_FLOAT_VOLT, channel.index-1);
            }

            if (isPMonData(cursor, id)) {
                return Value(channelSnapshot.pMon, VALUE_TYPE_FLOAT_WATT, channel.index-1);
            }

            if (id == DATA_ID_LRIP) {
                return Value(channel.flags.lrippleEnabled ? 1 : 0);
            }

            if (id == DATA_ID_CHANNEL_RPROG_STATUS) {
                return Value(channel.flags.rprogEnabled ? 1 : 0);
            }

            if (id == DATA_ID_OVP) {
                unsigned ovp;
                if (!channel.prot_conf.flags.u_state) ovp = 0;
                else if (!channel.ovp.flags.tripped) ovp = 1;
                else ovp = 2;
                return Value(ovp);
            }
        
            if (id == DATA_ID_OCP) {
                unsigned ocp;
                if (!channel.prot_conf.flags.i_state) ocp = 0;
                else if (!channel.ocp.flags.tripped) ocp = 1;
                else ocp = 2;
                return Value(ocp);
            }
        
            if (id == DATA_ID_OPP) {
                unsigned opp;
                if (!channel.prot_conf.flags.p_state) opp = 0;
                else if (!channel.opp.flags.tripped) opp = 1;
                else opp = 2;
                return Value(opp);
            }
        
            if (id == DATA_ID_OTP) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
                return 0;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
                temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + channel.index - 1];
                if (!tempSensor.isInstalled() || !tempSensor.isTestOK() || !tempSensor.prot_conf.state) return 0;
                else if (!tempSensor.isTripped()) return 1;
                else return 2;
#endif
            }
        
            if (id == DATA_ID_CHANNEL_LABEL) {
                return data::Value(channel.index, VALUE_TYPE_CHANNEL_LABEL);
            }
        
            if (id == DATA_ID_CHANNEL_SHORT_LABEL) {
                return data::Value(channel.index, VALUE_TYPE_CHANNEL_SHORT_LABEL);
            }

            if (id == DATA_ID_CHANNEL_TEMP_STATUS) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
                return 2;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
                temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + channel.index - 1];
                if (tempSensor.isInstalled()) return tempSensor.isTestOK() ? 1 : 0;
                else return 2;
#endif
            }

            if (id == DATA_ID_CHANNEL_TEMP) {
                float temperature = 0;
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
                temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + channel.index - 1];
                if (tempSensor.isInstalled() && tempSensor.isTestOK()) {
                    temperature = tempSensor.temperature;
                }
#endif
                return data::Value(temperature, VALUE_TYPE_FLOAT_CELSIUS);
            }

            if (id == DATA_ID_CHANNEL_ON_TIME_TOTAL) {
                return data::Value((uint32_t)channel.onTimeCounter.getTotalTime(), VALUE_TYPE_ON_TIME_COUNTER);
            }

            if (id == DATA_ID_CHANNEL_ON_TIME_LAST) {
                return data::Value((uint32_t)channel.onTimeCounter.getLastTime(), VALUE_TYPE_ON_TIME_COUNTER);
            }

            if (id == DATA_ID_CHANNEL_HAS_SUPPORT_FOR_CURRENT_DUAL_RANGE) {
                return data::Value(channel.hasSupportForCurrentDualRange() ? 1 : 0);
            }

            if (id == DATA_ID_CHANNEL_LIST_COUNTDOWN) {
                int32_t remaining;
                uint32_t total;
                if (list::getCurrentDwellTime(channel, remaining, total) && total >= 5000000L) {
                    if (remaining > 0) {
                        return Value((uint32_t)(remaining / 1000000L), VALUE_TYPE_COUNTDOWN);
                    } else {
                        return Value(0, VALUE_TYPE_COUNTDOWN);
                    }
                } else {
                    return Value();
                }
            }
        }
    }

    if (id == DATA_ID_CHANNEL_IS_VOLTAGE_BALANCED) {
        if (channel_dispatcher::isSeries()) {
            return Channel::get(0).isVoltageBalanced() || Channel::get(1).isVoltageBalanced() ? 1 : 0;
        } else {
            return 0;
        }
    }

    if (id == DATA_ID_CHANNEL_IS_CURRENT_BALANCED) {
        if (channel_dispatcher::isParallel()) {
            return Channel::get(0).isCurrentBalanced() || Channel::get(1).isCurrentBalanced() ? 1 : 0;
        } else {
            return 0;
        }
    }

    if (id == DATA_ID_OTP) {
        temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::AUX];
        if (!tempSensor.prot_conf.state) return 0;
        else if (!tempSensor.isTripped()) return 1;
        else return 2;
    }

    if (id == DATA_ID_SYS_PASSWORD_IS_SET) {
        return data::Value(strlen(persist_conf::devConf2.systemPassword) > 0 ? 1 : 0);
    }

    if (id == DATA_ID_SYS_RL_STATE) {
        return data::Value(g_rlState);
    }

    if (id == DATA_ID_SYS_TEMP_AUX_STATUS) {
        temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::AUX];
        if (tempSensor.isInstalled()) return tempSensor.isTestOK() ? 1 : 0;
        else return 2;
    }

    if (id == DATA_ID_SYS_TEMP_AUX) {
        float auxTemperature = 0;
        temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::AUX];
        if (tempSensor.isInstalled() && tempSensor.isTestOK()) {
            auxTemperature = tempSensor.temperature;
        }
        return data::Value(auxTemperature, VALUE_TYPE_FLOAT_CELSIUS);
    }

    if (id == DATA_ID_ALERT_MESSAGE) {
        return g_alertMessage;
    }
    
    if (id == DATA_ID_ALERT_MESSAGE_2) {
        return g_alertMessage2;
    }

    if (id == DATA_ID_ALERT_MESSAGE_3) {
        return g_alertMessage3;
    }

    if (id == DATA_ID_MODEL_INFO) {
        return Value(getModelInfo());
    }
    
    if (id == DATA_ID_FIRMWARE_INFO) {
        return Value(getFirmwareInfo());
    }

    if (id == DATA_ID_SYS_ETHERNET_INSTALLED) {
        return data::Value(OPTION_ETHERNET);
    }

    if (id == DATA_ID_SYS_ETHERNET_IS_CONNECTED) {
        return data::Value(ethernet::isConnected());
    }

    if (id == DATA_ID_SYS_ENCODER_INSTALLED) {
        return data::Value(OPTION_ENCODER);
    }

    if (id == DATA_ID_SYS_DISPLAY_STATE) {
        return data::Value(persist_conf::devConf2.flags.displayState);
    }

    if (id == DATA_ID_TEXT_MESSAGE) {
        return data::Value(getTextMessageVersion(), VALUE_TYPE_TEXT_MESSAGE);
    }

    if (id == DATA_ID_SERIAL_IS_CONNECTED) {
        return data::Value(serial::isConnected());
    }

    Page *page = getActivePage();
    if (page) {
        Value value = page->getData(cursor, id);
        if (value.getType() != VALUE_TYPE_NONE) {
            return value;
        }
    }

    page = getPreviousPage();
    if (page) {
        Value value = page->getData(cursor, id);
        if (value.getType() != VALUE_TYPE_NONE) {
            return value;
        }
    }

    Keypad *keypad = getActiveKeypad();
    if (keypad) {
        Value value = keypad->getData(id);
        if (value.getType() != VALUE_TYPE_NONE) {
            return value;
        }
    }

    Value value;

    value = edit_mode::getData(cursor, id);
    if (value.getType() != VALUE_TYPE_NONE) {
        return value;
    }

    value = gui::calibration::getData(cursor, id);
    if (value.getType() != VALUE_TYPE_NONE) {
        return value;
    }

    return Value();
}

bool set(const Cursor &cursor, uint8_t id, Value value, int16_t *error) {
    if (id == DATA_ID_CHANNEL_U_SET || id == DATA_ID_CHANNEL_U_EDIT) {
        if (!util::between(value.getFloat(), channel_dispatcher::getUMin(Channel::get(cursor.i)), channel_dispatcher::getUMax(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_VOLT, cursor.i)) {
            if (error) *error = SCPI_ERROR_DATA_OUT_OF_RANGE;
            return false;
        }
        
        if (util::greater(value.getFloat(), channel_dispatcher::getULimit(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_VOLT, cursor.i)) {
            if (error) *error = SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED;
            return false;
        }
        
        if (util::greater(value.getFloat() * channel_dispatcher::getISet(Channel::get(cursor.i)), channel_dispatcher::getPowerLimit(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_WATT, cursor.i)) {
            if (error) *error = SCPI_ERROR_POWER_LIMIT_EXCEEDED;
            return false;
        }
        
        channel_dispatcher::setVoltage(Channel::get(cursor.i), value.getFloat());

        return true;
    } else if (id == DATA_ID_CHANNEL_I_SET || id == DATA_ID_CHANNEL_I_EDIT) {
        if (!util::between(value.getFloat(), channel_dispatcher::getIMin(Channel::get(cursor.i)), channel_dispatcher::getIMax(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_AMPER, cursor.i)) {
            if (error) *error = SCPI_ERROR_DATA_OUT_OF_RANGE;
            return false;
        }
        
        if (util::greater(value.getFloat(), channel_dispatcher::getILimit(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_AMPER, cursor.i)) {
            if (error) *error = SCPI_ERROR_CURRENT_LIMIT_EXCEEDED;
            return false;
        }
        
        if (util::greater(value.getFloat() * channel_dispatcher::getUSet(Channel::get(cursor.i)), channel_dispatcher::getPowerLimit(Channel::get(cursor.i)), VALUE_TYPE_FLOAT_VOLT, cursor.i)) {
            if (error) *error = SCPI_ERROR_POWER_LIMIT_EXCEEDED;
            return false;
        }
        
        channel_dispatcher::setCurrent(Channel::get(cursor.i), value.getFloat());

        return true;
    } else if (id == DATA_ID_ALERT_MESSAGE) {
        g_alertMessage = value;
        return true;
    } else if (id == DATA_ID_ALERT_MESSAGE_2) {
        g_alertMessage2 = value;
        return true;
    } else if (id == DATA_ID_ALERT_MESSAGE_3) {
        g_alertMessage3 = value;
        return true;
    } else if (id == DATA_ID_EDIT_STEPS) {
        edit_mode_step::setStepIndex(value.getInt());
        return true;
    }

    Page *activePage = getActivePage();
    if (activePage && activePage->setData(cursor, id, value)) {
        return true;
    }

    if (error) *error = 0;
    return false;
}

int getNumHistoryValues(uint8_t id) {
    return CHANNEL_HISTORY_SIZE;
}

int getCurrentHistoryValuePosition(const Cursor &cursor, uint8_t id) {
    return Channel::get(cursor.i).getCurrentHistoryValuePosition();
}

Value getHistoryValue(const Cursor &cursor, uint8_t id, int position) {
    if (isUMonData(cursor, id)) {
        return Value(channel_dispatcher::getUMonHistory(Channel::get(cursor.i), position), VALUE_TYPE_FLOAT_VOLT, cursor.i);
    } else if (isIMonData(cursor, id)) {
        return Value(channel_dispatcher::getIMonHistory(Channel::get(cursor.i), position), VALUE_TYPE_FLOAT_AMPER, cursor.i);
    } else if (isPMonData(cursor, id)) {
        float pMon = util::multiply(
            channel_dispatcher::getUMonHistory(Channel::get(cursor.i), position),
            channel_dispatcher::getIMonHistory(Channel::get(cursor.i), position),
            getPrecision(VALUE_TYPE_FLOAT_WATT));
        return Value(pMon, VALUE_TYPE_FLOAT_WATT, cursor.i);
    }
    return Value();
}

bool isBlinking(const Cursor &cursor, uint8_t id) {
    bool result;
    if (edit_mode::isBlinking(cursor, id, result)) {
        return result;
    }

    if (id == DATA_ID_TRIGGER_IS_INITIATED) {
        return trigger::isInitiated();
    }

    if (g_focusCursor == cursor && g_focusDataId == id && g_focusEditValue.getType() != VALUE_TYPE_NONE) {
        return true;
    }

    return false;
}

Value getEditValue(const Cursor &cursor, uint8_t id) {
    int iChannel;
    if (cursor.i >= 0) {
        iChannel = cursor.i;
    } else if (g_channel) {
        iChannel = g_channel->index - 1;
    } else {
        iChannel = 0;
    }

    Channel &channel = Channel::get(iChannel);

    if (id == DATA_ID_CHANNEL_U_SET || id == DATA_ID_CHANNEL_U_EDIT) {
        return Value(channel_dispatcher::getUSetUnbalanced(channel), VALUE_TYPE_FLOAT_VOLT, iChannel);
    }
        
    if (id == DATA_ID_CHANNEL_I_SET || id == DATA_ID_CHANNEL_I_EDIT) {
        return Value(channel_dispatcher::getISetUnbalanced(channel), VALUE_TYPE_FLOAT_AMPER, iChannel);
    }
        
    return get(cursor, id);
}

}
}
}
} // namespace eez::psu::ui::data

#endif