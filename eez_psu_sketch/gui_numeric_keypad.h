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

#include "gui_internal.h"

namespace eez {
namespace psu {
namespace gui {
namespace numeric_keypad {

struct Options {
	data::ValueType editUnit;

	float min;
	float max;
	float def;

	struct {
		unsigned genericNumberKeypad : 1;
		unsigned maxButtonEnabled : 1;
		unsigned defButtonEnabled : 1;
		unsigned signButtonEnabled : 1;
		unsigned dotButtonEnabled : 1;
	} flags;
};

void init(const char *label, Options &options, void (*ok)(float), void (*cancel)());
// data::ValueType editUnit, float min, float max, bool maxButtonEnabled, float def, bool defButtonEnabled         bool genericNumberKeypad
void start(const char *label, Options &options, void (*ok)(float), void (*cancel)() = 0);

bool isEditing();

data::ValueType getEditUnit();
bool getText(char *text, int count);
data::Value getData(uint8_t id);
void switchToMilli();

void key(char c);
void space();
void caps();
void back();
void clear();
void sign();
void unit();
void setMaxValue();
void setDefaultValue();
void ok();
void cancel();

}
}
}
} // namespace eez::psu::gui::numeric_keypad
