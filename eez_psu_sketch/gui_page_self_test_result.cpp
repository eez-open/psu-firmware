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

#if OPTION_DISPLAY

#include "devices.h"

#include "gui_data_snapshot.h"
#include "gui_page_self_test_result.h"

namespace eez {
namespace psu {
namespace gui {

void SelfTestResultPage::pageWillAppear() {
	selfTestResult = devices::getSelfTestResultString();
}

void SelfTestResultPage::pageDidDisappear() {
	free(selfTestResult);
}

data::Value SelfTestResultPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_SELF_TEST_RESULT) {
		return data::Value(selfTestResult);
	}

	return data::Value();
}

}
}
} // namespace eez::psu::gui

#endif