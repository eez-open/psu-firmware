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

#include "gui_internal.h"
#include "gui_keypad.h"
#include "gui_numeric_keypad.h"
#include "gui_edit_mode.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

NumericKeypadOptions::NumericKeypadOptions()
{
    flags.option1ButtonEnabled = false;
    flags.option2ButtonEnabled = false;
    flags.signButtonEnabled = false;
    flags.dotButtonEnabled = false;
}

void NumericKeypadOptions::enableMaxButton() {
	flags.option1ButtonEnabled = true;
    option1ButtonText = PSTR("max");
    option1 = maxOption;
}

void NumericKeypadOptions::enableDefButton() {
	flags.option2ButtonEnabled = true;
    option2ButtonText = PSTR("def");
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

	if (m_options.flags.genericNumberKeypad) {
		m_maxChars = 16;
	}

	reset();
}

NumericKeypad *NumericKeypad::start(const char *label, const data::Value& value, NumericKeypadOptions &options, void (*ok)(float), void (*cancel)()) {
    NumericKeypad *page = new NumericKeypad();
	
    page->init(label, value, options, ok, cancel);

    pushPage(PAGE_ID_NUMERIC_KEYPAD, page);
    
    return page;
}

data::Value NumericKeypad::getData(uint8_t id) {
    data::Value value = Keypad::getData(id);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

    if (id == DATA_ID_EDIT_UNIT) {
        switch (getEditUnit()) {
        case data::VALUE_TYPE_FLOAT_VOLT:         return data::Value::ProgmemStr(PSTR("mV")); break;
        case data::VALUE_TYPE_FLOAT_MILLI_VOLT:   return data::Value::ProgmemStr(PSTR("V"));  break;
        case data::VALUE_TYPE_FLOAT_AMPER:        return data::Value::ProgmemStr(PSTR("mA")); break;
	    case data::VALUE_TYPE_FLOAT_MILLI_AMPER:  return data::Value::ProgmemStr(PSTR("A"));  break;
        case data::VALUE_TYPE_FLOAT_SECOND:       return data::Value::ProgmemStr(PSTR("ms"));  break;
	    case data::VALUE_TYPE_FLOAT_MILLI_SECOND: return data::Value::ProgmemStr(PSTR("s")); break;
        default:                                  return data::Value::ProgmemStr(PSTR(""));
        }
    }

    if (id == DATA_ID_KEYPAD_OPTION1_ENABLED) {
		return data::Value(m_options.flags.option1ButtonEnabled ? 1 : 0);
    }
    
	if (id == DATA_ID_KEYPAD_OPTION1_TEXT) {
		return data::Value(m_options.flags.option1ButtonEnabled ? m_options.option1ButtonText : PSTR(""));
    }

    if (id == DATA_ID_KEYPAD_OPTION2_ENABLED) {
		return data::Value(m_options.flags.option2ButtonEnabled ? 1 : 0);
	}
    
	if (id == DATA_ID_KEYPAD_OPTION2_TEXT) {
		return data::Value(m_options.flags.option2ButtonEnabled ? m_options.option2ButtonText : PSTR(""));
    }

    if (id == DATA_ID_KEYPAD_DOT_ENABLED) {
		return data::Value(m_options.flags.dotButtonEnabled ? 1 : 0);
	}
    
    if (id == DATA_ID_KEYPAD_SIGN_ENABLED) {
		return data::Value(m_options.flags.signButtonEnabled ? 1 : 0);
	}
    
    if (id == DATA_ID_KEYPAD_UNIT_ENABLED) {
		return data::Value(
			m_options.editUnit == data::VALUE_TYPE_FLOAT_VOLT ||
			m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT ||
			m_options.editUnit == data::VALUE_TYPE_FLOAT_AMPER ||
			m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER ||
            m_options.editUnit == data::VALUE_TYPE_FLOAT_SECOND ||
			m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_SECOND ? 1 : 0);
	}

	return data::Value();
}

bool NumericKeypad::isEditing() {
    return m_state != EMPTY && m_state != START;
}

data::ValueType NumericKeypad::getEditUnit() {
    return m_options.editUnit;
}

char NumericKeypad::getDotSign() {
	if (m_options.editUnit == data::VALUE_TYPE_TIME_ZONE) {
		return ':';
	}
	return '.';
}

void NumericKeypad::appendEditUnit(char *text) {
	// TODO move this to data::Value
	if (m_options.editUnit == data::VALUE_TYPE_FLOAT_VOLT) {
		strcat_P(text, PSTR("V"));
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT) {
		strcat_P(text, PSTR("mV"));
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_AMPER) {
		strcat_P(text, PSTR("A"));
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER) {
		strcat_P(text, PSTR("mA"));
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_WATT) {
		strcat_P(text, PSTR("W"));
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_SECOND) {
		strcat_P(text, PSTR("s"));
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_SECOND) {
		strcat_P(text, PSTR("ms"));
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_CELSIUS) {
		strcat_P(text, PSTR("oC"));
	}
}

void NumericKeypad::getKeypadText(char *text) {
	if (m_label) {
		strcpy_P(text, m_label);
		text += strlen(m_label);
	}

    getText(text, 16);
}

bool NumericKeypad::getText(char *text, int count) {
	if (m_options.flags.genericNumberKeypad) {
		if (m_state == START) {
            m_startValue.toText(text, count);
            return false;
		}
        
        strcpy(text, m_keypadText);
	} else {
		if (m_state == START) {
			*text = 0;

            edit_mode::getCurrentValue().toText(text, count);

			return false;
		}

		int i = 0;

		if (m_state >= D0 && (m_d0 != 0 || m_state < DOT)) {
			text[i++] = m_d0 + '0';
		}

		if (m_state >= D1) {
			text[i++] = m_d1 + '0';
		}

		if (!isMilli() && m_state >= DOT) {
			text[i++] = getDotSign();
		}

		if (m_state >= D2) {
			text[i++] = m_d2 + '0';
		}

		if (m_state >= D3) {
			text[i++] = m_d3 + '0';
		}

		text[i] = 0;
	}

	appendCursor(text);

    appendEditUnit(text);

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool NumericKeypad::isMilli() {
	return m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT ||
        m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER ||
        m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_SECOND;
}

void NumericKeypad::switchToMilli() {
	if (m_options.editUnit == data::VALUE_TYPE_FLOAT_VOLT) {
		m_options.editUnit = data::VALUE_TYPE_FLOAT_MILLI_VOLT;
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_AMPER) {
		m_options.editUnit = data::VALUE_TYPE_FLOAT_MILLI_AMPER;
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_SECOND) {
		m_options.editUnit = data::VALUE_TYPE_FLOAT_MILLI_SECOND;
	}
}

float NumericKeypad::getValue() {
	if (m_options.flags.genericNumberKeypad) {
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
	} else {
		float value = 0;

		if (m_state >= D0) {
			value = m_d0 * 1.0f;
		}

		if (m_state >= D1) {
			value = value * 10 + m_d1;
		}

		if (isMilli()) {
			if (m_state >= D2) {
				value = value * 10 + m_d2;
			}

			value /= 1000.0f;
		}
		else {
			value *= 100.0f;

			if (m_state >= D2) {
				value += m_d2 * 10;
			}

			if (m_state >= D3) {
				value += m_d3;
			}

			value /= 100.0f;
		}

		return value;
	}
}

bool NumericKeypad::setValue(float fvalue) {
	if (m_options.flags.genericNumberKeypad) {
		return true;
	} else {
		if (isMilli()) {
			long value = (long)floor(fvalue * 1000);
        
			if (value > 999) {
				return false;
			}

			if (value >= 100) {
				m_d0 = value / 100;
				value = value % 100;
				m_d1 = value / 10;
				m_d2 = value % 10;
				m_state = D2;
			} else if (value >= 10) {
				m_d0 = value / 10;
				m_d1 = value % 10;
				m_state = D1;
			} else {
				m_d0  = value;
				m_state = D0;
			}
		} else {
			long value = (long)round(fvalue * 100);
        
			m_d3 = value % 10;
			if (m_d3 != 0) {
				m_state = D3;
            
				value /= 10;
				m_d2 = value % 10;
            
				value /= 10;
				m_d1 = value % 10;

				value /= 10;
				m_d0 = value % 10;
			} else {
				value /= 10;
				m_d2 = value % 10;
				if (m_d2 != 0) {
					m_state = D2;

					value /= 10;
					m_d1 = value % 10;

					value /= 10;
					m_d0 = value % 10;
				} else {
					value /= 10;
					m_d1 = value % 10;

					m_d0 = value / 10;
					if (m_d0 == 0) {
						m_d0 = m_d1;
						m_state = D0;
					} else {
						m_state = D1;
					}
				}
			}
		}

		return true;
	}
}

bool NumericKeypad::isValueValid() {
	if (m_state == EMPTY) {
		return false;
	}
	float value = getValue();
	return (value >= m_options.min && value <= m_options.max);
}

void NumericKeypad::digit(int d) {
	if (m_options.flags.genericNumberKeypad) {
		if (m_state == START || m_state == EMPTY) {
			m_state = BEFORE_DOT;
			if (m_options.editUnit == data::VALUE_TYPE_TIME_ZONE) {
				if (strlen(m_keypadText) == 0) {
					appendChar('+');
				}
			}
		}
		appendChar(d + '0');
	} else {
		if (m_state == START || m_state == EMPTY) {
			m_d0 = d;
			m_state = D0;
			/*
			if (!isValueValid()) {
				toggleEditUnit();
			} 
			if (!isValueValid()) {
				reset();
				sound::playBeep();
			}
			*/
		}
		else if (m_state == D0) {
			m_d1 = d;
			m_state = D1;
			/*
			if (!isValueValid()) {
				g_state = D0;
				sound::playBeep();
			}
			*/
		}
		else if (m_state == DOT || (isMilli() && m_state == D1)) {
			NumericKeypadState saved_state = m_state;
			m_d2 = d;
			m_state = D2;
			/*
			if (!isValueValid()) {
				g_state = saved_state;
				sound::playBeep();
			}
			*/
		}
		else if (m_state == D2 && !isMilli()) {
			m_d3 = d;
			m_state = D3;
			/*
			if (!isValueValid()) {
				g_state = D2;
				sound::playBeep();
			}
			*/
		}
		else {
			sound::playBeep();
		}
	}
}

void NumericKeypad::dot() {
	if (m_options.flags.genericNumberKeypad) {
		if (!m_options.flags.dotButtonEnabled) {
			return;
		}

		if (m_state == EMPTY) {
			if (m_options.editUnit == data::VALUE_TYPE_TIME_ZONE) {
				if (strlen(m_keypadText) == 0) {
					appendChar('+');
				}
			}
			appendChar('0');
			m_state = BEFORE_DOT;
		}

		if (m_state == BEFORE_DOT) {
			appendChar(getDotSign());
			m_state = AFTER_DOT;
		} else {
			sound::playBeep();
		}
	} else {
		if (isMilli()) {
			sound::playBeep();
		} else {
			if (m_state == START || m_state == EMPTY) {
				m_d0 = 0;
				m_d1 = 0;
				m_state = DOT;
			}
			else if (m_state == D0) {
				m_d1 = m_d0;
				m_d0 = 0;
				m_state = DOT;
			}
			else if (m_state == D1) {
				m_state = DOT;
			}
			else {
				sound::playBeep();
			}
		}
	}
}

void NumericKeypad::toggleEditUnit() {
	if (m_options.editUnit == data::VALUE_TYPE_FLOAT_VOLT) {
		m_options.editUnit = data::VALUE_TYPE_FLOAT_MILLI_VOLT;
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT) {
		m_options.editUnit = data::VALUE_TYPE_FLOAT_VOLT;
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_AMPER) {
		m_options.editUnit = data::VALUE_TYPE_FLOAT_MILLI_AMPER;
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER) {
		m_options.editUnit = data::VALUE_TYPE_FLOAT_AMPER;
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_SECOND) {
		m_options.editUnit = data::VALUE_TYPE_FLOAT_MILLI_SECOND;
	} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_SECOND) {
		m_options.editUnit = data::VALUE_TYPE_FLOAT_SECOND;
	}
}

void NumericKeypad::reset() {
	if (m_options.flags.genericNumberKeypad) {
		m_state = m_startValue.getType() != data::VALUE_TYPE_NONE ? START : EMPTY;
		m_keypadText[0] = 0;
	} else {
		m_state = START;

		if (m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_VOLT) {
			m_options.editUnit = data::VALUE_TYPE_FLOAT_VOLT;
		} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_AMPER) {
			m_options.editUnit = data::VALUE_TYPE_FLOAT_AMPER;
		} else if (m_options.editUnit == data::VALUE_TYPE_FLOAT_MILLI_SECOND) {
			m_options.editUnit = data::VALUE_TYPE_FLOAT_SECOND;
		}
	}
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
	if (m_options.flags.genericNumberKeypad) {
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
	} else {
		if (m_state == D3) {
			m_state = D2;
		}
		else if (m_state == D2) {
			if (isMilli()) {
				m_state = D1;
			}
			else {
				m_state = DOT;
			}
		}
		else if (m_state == DOT) {
			if (m_d0 == 0) {
				m_d0 = m_d1;
				m_state = D0;
			}
			else {
				m_state = D1;
			}
		}
		else if (m_state == D1) {
			m_state = D0;
		}
		else if (m_state == D0) {
			m_state = EMPTY;
		}
		else if (m_state == START) {
            m_state = EMPTY;
        } else {
			sound::playBeep();
		}
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
		if (m_options.flags.genericNumberKeypad && m_options.editUnit == data::VALUE_TYPE_TIME_ZONE) {
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
	if (m_options.flags.genericNumberKeypad) {
        if (m_state == START) {
            m_state = EMPTY;
        }
		toggleEditUnit();
	} else {
		float value = getValue();
		data::ValueType savedEditUnit = m_options.editUnit;

		toggleEditUnit();

		if (m_state == START) {
			m_state = EMPTY;
		} else {
			if (m_state != EMPTY && !setValue(value)) {
				m_options.editUnit = savedEditUnit;
				sound::playBeep();
			}
		}
	}
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
	if (m_options.flags.genericNumberKeypad) {
        if (m_state == START) {
            ((void (*)(float))m_okCallback)(m_startValue.isFloat() ? m_startValue.getFloat() : m_startValue.getInt());

            return;
        } 
        
        if (m_state != EMPTY) {
			float value = getValue();

			if (util::less(value, m_options.min, data::getPrecision(m_options.editUnit))) {
				errorMessage(0, data::Value::LessThenMinMessage(m_options.min, m_options.editUnit));
			} else if (util::greater(value, m_options.max, data::getPrecision(m_options.editUnit))) {
				errorMessage(0, data::Value::GreaterThenMaxMessage(m_options.max, m_options.editUnit));
			} else {
				((void (*)(float))m_okCallback)(value);
			}

            return;
        }
	} else if (m_state != START && m_state != EMPTY) {
		if (((bool (*)(float))m_okCallback)(getValue())) {
			reset();
		}

        return;
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

bool NumericKeypad::onEncoder(int counter) {
    if (m_options.flags.genericNumberKeypad) {
        if (m_state == START) {
            if (data::isFloatType(getEditUnit())) {
                encoder::enableAcceleration(true);
                encoder::setMovingSpeedMultiplier(m_options.max / data::getMax(0, DATA_ID_CHANNEL_U_SET).getFloat());

                float newValue = m_startValue.getFloat() + 0.01f * counter;

                if (newValue < m_options.min) {
                    newValue = m_options.min;
                }

                if (newValue > m_options.max) {
                    newValue = m_options.max;
                }

                m_startValue = data::Value(newValue, m_startValue.getType());

                return true;
            } else if (m_startValue.getType() == data::VALUE_TYPE_INT) {
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
    } else {
        if (m_state == START) {
            return false;
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