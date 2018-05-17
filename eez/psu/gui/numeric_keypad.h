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

#pragma once

#include "eez/psu/gui/keypad.h"

namespace eez {
namespace psu {
namespace gui {

struct NumericKeypadOptions {
    NumericKeypadOptions();

    int channelIndex;

	Unit editValueUnit;
    int numSignificantDecimalDigits;

	float min;
	float max;
	float def;

	struct {
        unsigned checkWhileTyping: 1;
		unsigned option1ButtonEnabled : 1;
		unsigned option2ButtonEnabled : 1;
		unsigned signButtonEnabled : 1;
		unsigned dotButtonEnabled : 1;
	} flags;

    const char *option1ButtonText;
    void (*option1)();

    const char *option2ButtonText;
    void (*option2)();

    void enableMaxButton();
    void enableDefButton();

private:
    static void maxOption();
    static void defOption();
};

enum NumericKeypadState {
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

class NumericKeypad : public Keypad {
public:
    void init(const char *label, const eez::mw::gui::data::Value& value, NumericKeypadOptions &options, void (*ok)(float), void (*cancel)());
    static NumericKeypad *start(const char *label, const eez::mw::gui::data::Value& value, NumericKeypadOptions &options, void (*ok)(float), void (*cancel)() = 0);

    bool isEditing();

    Unit getEditUnit();
    Unit getValueUnit();
    void switchToMilli();

    void getKeypadText(char *text);
    bool getText(char *text, int count);

    void key(char ch);
    void space();
    void caps();
    void back();
    void clear();
    void sign();
    void unit();
    void option1();
    void option2();
    void setMaxValue();
    void setDefValue();
    void ok();
    void cancel();

#if OPTION_ENCODER
    bool onEncoderClicked();
    bool onEncoder(int counter);
#endif

	Unit getSwitchToUnit();

	NumericKeypadOptions m_options;

private:
	eez::mw::gui::data::Value m_startValue;
    NumericKeypadState m_state;

    void appendEditUnit(char *text);
    float getValue();
    char getDotSign();
    bool isMilli();
    Unit getMilliUnit();
    void toggleEditUnit();
    int getNumDecimalDigits();
    bool isValueValid();
    bool checkNumSignificantDecimalDigits();
    void digit(int d);
    void dot();
    void reset();
};

}
}
} // namespace eez::psu::gui
