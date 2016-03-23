/*
* EEZ PSU Firmware
* Copyright (C) 2015 Envox d.o.o.
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

#include "gui_data.h"

namespace eez {
namespace psu {
namespace gui {

namespace data {
    struct Snapshot;
}

namespace edit_mode {

struct Snapshot {
    data::Value editValue;
    char infoText[32];
    int interactiveModeSelector;

    int step_index;

    data::Value keypadUnit;
    char keypadText[10];

    void takeSnapshot();
    data::Value get(uint8_t id);
    bool isBlinking(data::Snapshot& snapshot, uint8_t id, bool &result);
};

}
}
}
} // namespace eez::psu::ui::edit_mode
