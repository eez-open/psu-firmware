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

namespace protection {

void clear();
void clearAndDisable();

void editOVP();
void editOCP();
void editOPP();
void editOTP();

int getState();
int getDirty();
data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);

void toggleState();
void editLimit();
void editLevel();
void editDelay();
void set();
void discard();

}
}
}
} // namespace eez::psu::gui::protection
