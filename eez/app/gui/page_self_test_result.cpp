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

#include "eez/app/psu.h"

#if OPTION_DISPLAY

#include "eez/app/devices.h"

#include "eez/app/gui/page_self_test_result.h"

namespace eez {
namespace app {
namespace gui {

void SelfTestResultPage::pageWillAppear() {
	m_selfTestResult = devices::getSelfTestResultString();
}

void SelfTestResultPage::pageDidDisappear() {
	free(m_selfTestResult);
}

}
}
} // namespace eez::app::gui

#endif