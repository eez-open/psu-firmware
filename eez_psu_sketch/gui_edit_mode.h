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

#include "gui_internal.h"
#include "gui_data_snapshot.h"

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode {

bool isActive();
void enter(int tab_index_ = -1);
void exit();
bool isInteractiveMode();
void toggleInteractiveMode();

const data::Value& getEditValue();
data::Value getCurrentValue(data::Snapshot snapshot);
const data::Value& getMin();
const data::Value& getMax();
data::Unit getUnit();
void setValue(float value);

bool isEditWidget(const WidgetCursor &widgetCursor);
void getInfoText(char *infoText);

void nonInteractiveSet();
void nonInteractiveDiscard();

}
}
}
} // namespace eez::psu::gui::edit_mode
