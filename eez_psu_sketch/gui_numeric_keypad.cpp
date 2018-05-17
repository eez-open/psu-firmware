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

#if OPTION_DISPLAY

#if OPTION_ENCODER
#include "encoder.h"
#endif
#include "sound.h"

#include "channel_dispatcher.h"

#include "gui_psu.h"
#include "gui_data.h"
#include "gui_keypad.h"
#include "gui_numeric_keypad.h"
#include "gui_edit_mode.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

NumericKeypadOptions::NumericKeypadOptions() {
    this->channelIndex = -1;

    numSignificantDecimalDigits = -1;

    flags.checkWhileTyping = false;
    flags.option1ButtonEnabled = false;
    flags.option2ButtonEnabled = false;
    flags.signButtonEnabled = false;
    flags.dotButtonEnabled = false;

	editValueUnit = UNIT_UNKNOWN;
}

void NumericKeypadOptions::enableMaxButton() {
    flags.option1ButtonEnabled = true;
    option1ButtonText = "max";
    option1 = maxOption;
}

void NumericKeypadOptions::enableDefButton() {
    flags.option2ButtonEnabled = true;
    option2ButtonText = "def";
    option2 = defOption;
}

void NumericKeypadOptions::maxOption() {
    getActiveKeypad()->setMaxValue();
}

void NumericKeypadOptions::defOption() {
    getActiveKeypad()->setDefValue();
}

////////////////////////////////////////////////////////////////////////////////

void NumericKeypad::init(const char *label, const data::Value& value, NumericKeypadOptions &options, void (*ok)(float), void (*cancel)()) {
    Keypad::init(label, (void (*)(char *))ok, cancel);

    m_startValue = value;

    m_options = options;

    if (value.getType() == VALUE_TYPE_IP_ADDRESS) {
        m_options.flags.dotButtonEnabled = true;
    }

    if (m_startValue.isMilli()) {
        switchToMilli();
    }

    m_maxChars = 16;

    reset();

    if (value.getType() == VALUE_TYPE_IP_ADDRESS) {
        ipAddressToString(value.getUInt32(), m_keypadText);
        m_state = BEFORE_DOT;
    }
}

NumericKeypad *NumericKeypad::start(const char *label, const data::Value& value, NumericKeypadOptions &options, void (*ok)(float), void (*cancel)()) {
    NumericKeypad *page = new NumericKeypad();

    page->init(label, value, options, ok, cancel);

    pushPage(PAGE_ID_NUMERIC_KEYPAD, page);

    return page;
}

bool NumericKeypad::isEditing() {
    return m_state != EMPTY && m_state != START;
}

char NumericKeypad::getDotSign() {
    if (m_startValue.getType() == VALUE_TYPE_TIME_ZONE) {
        return ':';
    }
    return '.';
}

void NumericKeypad::appendEditUnit(char *text) {
    strcat(text, getUnitName(m_options.editValueUnit));
}

void NumericKeypad::getKeypadText(char *text) {
    if (*m_label) {
        strcpy(text, m_label);
        text += strlen(m_label);
    }

    getText(text, 16);
}

bool NumericKeypad::getText(char *text, int count) {
    if (m_state == START) {
        if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD) {
            edit_mode::getCurrentValue().toText(text, count);
        } else {
            m_startValue.toText(text, count);
        }
        return false;
    }

    strcpy(text, m_keypadText);

    appendCursor(text);

    appendEditUnit(text);

    return true;
}

////////////////////////////////////////////////////////////////////////////////

Unit NumericKeypad::getEditUnit() {
    return m_options.editValueUnit;
}

Unit NumericKeypad::getValueUnit() {
    if (m_options.editValueUnit == UNIT_MILLI_VOLT)   return UNIT_VOLT;
    if (m_options.editValueUnit == UNIT_MILLI_AMPER)  return UNIT_AMPER;
    if (m_options.editValueUnit == UNIT_MILLI_SECOND) return UNIT_SECOND;
    if (m_options.editValueUnit == UNIT_MILLI_WATT) return UNIT_WATT;
    return m_options.editValueUnit;
}

bool NumericKeypad::isMilli() {
    return
        m_options.editValueUnit == UNIT_MILLI_VOLT ||
        m_options.editValueUnit == UNIT_MILLI_AMPER ||
        m_options.editValueUnit == UNIT_MILLI_WATT ||
        m_options.editValueUnit == UNIT_MILLI_SECOND;
}

Unit NumericKeypad::getMilliUnit() {
    if (m_options.editValueUnit == UNIT_VOLT)   return UNIT_MILLI_VOLT;
    if (m_options.editValueUnit == UNIT_AMPER)  return UNIT_MILLI_AMPER;
    if (m_options.editValueUnit == UNIT_WATT)   return UNIT_MILLI_WATT;
    if (m_options.editValueUnit == UNIT_SECOND) return UNIT_MILLI_SECOND;
    return m_options.editValueUnit;
}

void NumericKeypad::switchToMilli() {
    m_options.editValueUnit = getMilliUnit();
}

Unit NumericKeypad::getSwitchToUnit() {
    if (m_options.editValueUnit == UNIT_VOLT)         return UNIT_MILLI_VOLT;
    if (m_options.editValueUnit == UNIT_MILLI_VOLT)   return UNIT_VOLT;
    if (m_options.editValueUnit == UNIT_AMPER)        return UNIT_MILLI_AMPER;
    if (m_options.editValueUnit == UNIT_MILLI_AMPER)  return UNIT_AMPER;
    if (m_options.editValueUnit == UNIT_WATT)         return UNIT_MILLI_WATT;
    if (m_options.editValueUnit == UNIT_MILLI_WATT)   return UNIT_WATT;
    if (m_options.editValueUnit == UNIT_SECOND)       return UNIT_MILLI_SECOND;
    if (m_options.editValueUnit == UNIT_MILLI_SECOND) return UNIT_SECOND;
    return m_options.editValueUnit;
}

void NumericKeypad::toggleEditUnit() {
    m_options.editValueUnit = getSwitchToUnit();
}

////////////////////////////////////////////////////////////////////////////////

float NumericKeypad::getValue() {
    const char *p = m_keypadText;

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
}

int NumericKeypad::getNumDecimalDigits() {
    int n = 0;
    bool afterDot = false;
    for (int i = 0; m_keypadText[i]; ++i) {
        if (afterDot) {
            ++n;
        } else if (m_keypadText[i] == '.') {
            afterDot = true;
        }
    }
    return n;
}

bool NumericKeypad::isValueValid() {
    if (getActivePageId() != PAGE_ID_EDIT_MODE_KEYPAD) {
        return true;
    }

    float value = getValue();

    if (less(value, m_options.min, m_startValue.getUnit(), m_options.channelIndex) ||
        greater(value, m_options.max, m_startValue.getUnit(), m_options.channelIndex))
    {
        return false;
    }

    return true;
}

bool NumericKeypad::checkNumSignificantDecimalDigits() {
    if (getActivePageId() != PAGE_ID_EDIT_MODE_KEYPAD) {
        return true;
    }

    float value = getValue();
    Unit unit = getValueUnit();

    int numSignificantDecimalDigits = getNumSignificantDecimalDigits(unit);
    if (isMilli()) {
        numSignificantDecimalDigits -= 3;
        if (numSignificantDecimalDigits < 0) {
            numSignificantDecimalDigits = 0;
        }
    } else {
        if (mw::greater(value, 9.999f, 3)) {
            numSignificantDecimalDigits = 2;
        }
    }

    if (unit == UNIT_AMPER && mw::lessOrEqual(value, 0.5, getPrecision(UNIT_AMPER))) {
        ++numSignificantDecimalDigits;
    }

    return getNumDecimalDigits() <= numSignificantDecimalDigits;
}

void NumericKeypad::digit(int d) {
    if (m_state == START || m_state == EMPTY) {
        m_state = BEFORE_DOT;
        if (m_startValue.getType() == VALUE_TYPE_TIME_ZONE) {
            if (strlen(m_keypadText) == 0) {
                appendChar('+');
            }
        }
    }
    appendChar(d + '0');

    if (!checkNumSignificantDecimalDigits()) {
        back();
        sound::playBeep();
    }
}

void NumericKeypad::dot() {
    if (!m_options.flags.dotButtonEnabled) {
        return;
    }

    if (m_startValue.getType() == VALUE_TYPE_IP_ADDRESS) {
        if (m_state == EMPTY || m_state == START) {
            sound::playBeep();
        } else {
            appendChar(getDotSign());
        }
        return;
    }

    if (m_state == EMPTY) {
        if (m_startValue.getType() == VALUE_TYPE_TIME_ZONE) {
            if (strlen(m_keypadText) == 0) {
                appendChar('+');
            }
        }
        appendChar('0');
        m_state = BEFORE_DOT;
    }

    if (m_state == START || m_state == EMPTY) {
        appendChar('0');
        m_state = BEFORE_DOT;
    }

    if (m_state == BEFORE_DOT) {
        appendChar(getDotSign());
        m_state = AFTER_DOT;
    } else {
        sound::playBeep();
    }
}

void NumericKeypad::reset() {
    m_state = m_startValue.getType() != VALUE_TYPE_NONE ? START : EMPTY;
    m_keypadText[0] = 0;
}

void NumericKeypad::key(char ch) {
    if (ch >= '0' && ch <= '9') {
        digit(ch - '0');
    } else if (ch == '.') {
        dot();
    }
}

void NumericKeypad::space() {
    // DO NOTHING
}

void NumericKeypad::caps() {
    // DO NOTHING
}

void NumericKeypad::back() {
    int n = strlen(m_keypadText);
    if (n > 0) {
        if (m_keypadText[n - 1] == getDotSign()) {
            m_state = BEFORE_DOT;
        }
        m_keypadText[n - 1] = 0;
        if (n - 1 == 1) {
            if (m_keypadText[0] == '+' || m_keypadText[0] == '-') {
                m_state = EMPTY;
            }
        } else if (n - 1 == 0) {
            m_state = EMPTY;
        }
    } else if (m_state == START) {
        m_state = EMPTY;
    } else {
        sound::playBeep();
    }
}

void NumericKeypad::clear() {
    if (m_state != START) {
        reset();
    } else {
        sound::playBeep();
    }
}

void NumericKeypad::sign() {
    if (m_options.flags.signButtonEnabled) {
        if (m_startValue.getType() == VALUE_TYPE_TIME_ZONE) {
            if (m_keypadText[0] == 0) {
                m_keypadText[0] = '-';
                m_keypadText[1] = 0;
            } else if (m_keypadText[0] == '-') {
                m_keypadText[0] = '+';
            } else {
                m_keypadText[0] = '-';
            }
        } else {
            // not supported
            sound::playBeep();
        }
    }
}

void NumericKeypad::unit() {
    if (m_state == START) {
        m_state = EMPTY;
    }
    toggleEditUnit();
}

void NumericKeypad::option1() {
    if (m_options.flags.option1ButtonEnabled && m_options.option1) {
        m_options.option1();
    }
}

void NumericKeypad::option2() {
    if (m_options.flags.option2ButtonEnabled && m_options.option2) {
        m_options.option2();
    }
}

void NumericKeypad::setMaxValue() {
    ((void (*)(float))m_okCallback)(m_options.max);
}

void NumericKeypad::setDefValue() {
    ((void (*)(float))m_okCallback)(m_options.def);
}

void NumericKeypad::ok() {
    if (m_state == START) {
        if (m_startValue.getType() == VALUE_TYPE_IP_ADDRESS) {
            ((void (*)(uint32_t))m_okCallback)(m_startValue.getUInt32());
		} else if (m_startValue.getType() == VALUE_TYPE_TIME_ZONE) {
			((void(*)(float))m_okCallback)(m_startValue.getInt() / 100.0f);
		} else {
            ((void (*)(float))m_okCallback)(m_startValue.isFloat() ? m_startValue.getFloat() : m_startValue.getInt());
        }

        return;
    }

    if (m_state != EMPTY) {
        if (m_startValue.getType() == VALUE_TYPE_IP_ADDRESS) {
            uint32_t ipAddress;
            if (parseIpAddress(m_keypadText, strlen(m_keypadText), ipAddress)) {
                ((void (*)(uint32_t))m_okCallback)(ipAddress);
                m_state = START;
                m_keypadText[0] = 0;
            } else {
                errorMessageP("Invalid IP address format!");
            }

            return;
        } else {
            float value = getValue();

            if (less(value, m_options.min, m_startValue.getUnit(), m_options.channelIndex)) {
                errorMessage(0, MakeLessThenMinMessageValue(m_options.min, m_startValue));
            } else if (greater(value, m_options.max, m_startValue.getUnit(), m_options.channelIndex)) {
                errorMessage(0, MakeGreaterThenMaxMessageValue(m_options.max, m_startValue));
            } else {
                ((void (*)(float))m_okCallback)(value);
                m_state = START;
                m_keypadText[0] = 0;
                return;
            }

            return;
        }
    }

    sound::playBeep();
}

void NumericKeypad::cancel() {
    void (*cancel)() = m_cancelCallback;

    popPage();

    if (cancel) {
        cancel();
    }
}

#if OPTION_ENCODER

bool NumericKeypad::onEncoderClicked() {
    if (m_state == START) {
        if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD) {
            return false;
        }
    }
    ok();
    return true;
}

bool NumericKeypad::onEncoder(int counter) {
    if (m_state == START) {
        if (getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD) {
            return false;
        }

        if (m_startValue.getType() == VALUE_TYPE_FLOAT) {
            encoder::enableAcceleration(true);

            float newValue = m_startValue.getFloat() + 0.01f * counter;

            if (newValue < m_options.min) {
                newValue = m_options.min;
            }

            if (newValue > m_options.max) {
                newValue = m_options.max;
            }

            m_startValue = MakeValue(newValue, m_startValue.getUnit(), m_options.channelIndex);

            return true;
        } else if (m_startValue.getType() == VALUE_TYPE_INT) {
            encoder::enableAcceleration(false);

            int newValue = m_startValue.getInt() + counter;

            if (newValue < (int)m_options.min) {
                newValue = (int)m_options.min;
            }

            if (newValue > (int)m_options.max) {
                newValue = (int)m_options.max;
            }

            m_startValue = data::Value(newValue);

            return true;
        }
    }

    sound::playBeep();
    return true;
}

#endif

}
}
} // namespace eez::psu::gui

#endif