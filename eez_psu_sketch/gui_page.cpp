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

#include "gui_internal.h"
#include "gui_data_snapshot.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

void Page::pageWillAppear() {
}

void Page::takeSnapshot(data::Snapshot *snapshot) {
}

data::Value Page::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	return data::Value();
}

////////////////////////////////////////////////////////////////////////////////

void SetPage::takeSnapshot(data::Snapshot *snapshot) {
	snapshot->flags.setPageDirty = getDirty();
}

data::Value SetPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_SET_PAGE_DIRTY) {
		return data::Value(snapshot->flags.setPageDirty);
	}
	return data::Value();
}

void SetPage::edit() {
}

void SetPage::onSetValue(float value) {
	popPage();
	SetPage *page = (SetPage *)getActivePage();
	page->setValue(value);
}

void SetPage::setValue(float value) {
}

void SetPage::discard() {
	popPage();
}

}
}
} // namespace eez::psu::gui
