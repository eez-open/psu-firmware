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

#include "gui_page.h"

namespace eez {
namespace psu {
namespace gui {

class ChSettingsAdvPage : public Page {
public:
	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);
};

class ChSettingsAdvLRipplePage : public SetPage {
public:
	ChSettingsAdvLRipplePage();

	void takeSnapshot(data::Snapshot *snapshot);
	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);

	int getDirty();
	void set();

	void toggleStatus();
	void toggleAutoMode();

private:
	int origStatus;
	int status;

	int origAutoMode;
	int autoMode;
};

class ChSettingsAdvRSensePage : public Page {
public:
	void takeSnapshot(data::Snapshot *snapshot);
	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);

	void toggleStatus();
};

class ChSettingsAdvRProgPage : public Page {
public:
	void takeSnapshot(data::Snapshot *snapshot);
	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);

	void toggleStatus();
};

}
}
} // namespace eez::psu::gui
