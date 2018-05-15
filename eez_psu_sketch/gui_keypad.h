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

#include "mw_gui_page.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

class Keypad : public mw::gui::Page {
public:
    Keypad();

    static void startPush(const char *label, const char *text, int maxChars, bool isPassword, void (*ok)(char *), void (*cancel)());
    static void startReplace(const char *label, const char *text, int maxChars, bool isPassword, void (*ok)(char *), void (*cancel)());

    void key();
    virtual void key(char ch);
    virtual void space();
    virtual void caps();
    virtual void back();
    virtual void clear();
    virtual void sign();
    virtual void unit();
    virtual void option1();
    virtual void option2();
    virtual void setMaxValue();
    virtual void setDefValue();
    virtual void ok();
    virtual void cancel();

    virtual void getKeypadText(char *text);
	mw::gui::data::Value getKeypadTextValue();

    void appendChar(char c);
    void appendCursor(char *text);

	virtual Unit getSwitchToUnit() {
		return UNIT_UNKNOWN;
	}

	bool m_isUpperCase;

protected:
    char m_stateText[2][MAX_KEYPAD_TEXT_LENGTH + 2];
    char m_label[64];
    char m_keypadText[MAX_KEYPAD_TEXT_LENGTH + 2];
    int m_maxChars;
    void (*m_okCallback)(char *); // +2 for cursor and zero at the end
    void (*m_cancelCallback)();

    void init(const char *label, void (*ok)(char *), void (*cancel)());

private:
    bool m_isPassword;
    uint32_t m_lastKeyAppendTime;
    bool m_cursor;
    uint32_t m_lastCursorChangeTime;

    void start(const char *label, const char *text, int maxChars_, bool isPassword_, void (*ok)(char *), void (*cancel)());
};

Keypad *getActiveKeypad();

}
}
} // namespace eez::psu::gui
