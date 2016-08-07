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

namespace eez {
namespace psu {
namespace gui {

namespace data {
    struct Snapshot;
}

namespace keypad {

////////////////////////////////////////////////////////////////////////////////

struct Snapshot {
    char text[MAX_KEYPAD_TEXT_LENGTH + 2]; // +2 for cursor and zero at the end
	bool isUpperCase;
    data::Value keypadUnit;

    void takeSnapshot(data::Snapshot *snapshot);
	data::Value get(uint8_t id);
};

////////////////////////////////////////////////////////////////////////////////

extern void (*g_okCallback)(char *);
extern void (*g_cancelCallback)();

void init(const char *label, void (*ok)(char *), void (*cancel)());
void start(const char *label, const char *text, int maxChars, bool isPassword, void (*ok)(char *), void (*cancel)());

void key();
void space();
void caps();
void back();
void clear();
void sign();
void unit();
void ok();
void cancel();

void appendCursor(char *text);


}
}
}
} // namespace eez::psu::gui::keypad
