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

#pragma once

#include "mw_gui_data.h"

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode {

bool isActive();
void enter(int tabIndex = -1);
void update();
void exit();
bool isInteractiveMode();
void toggleInteractiveMode();

const eez::mw::gui::data::Value& getEditValue();
eez::mw::gui::data::Value getCurrentValue();
const eez::mw::gui::data::Value &getMin();
const eez::mw::gui::data::Value &getMax();
Unit getUnit();
bool setValue(float value);

void getInfoText(int part, char *infoText);

void nonInteractiveSet();
void nonInteractiveDiscard();

}
}
}
} // namespace eez::psu::gui::edit_mode
