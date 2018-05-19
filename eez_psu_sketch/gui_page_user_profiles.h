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

#include "mw_gui_page.h"
#include "profile.h"

namespace eez {
namespace app {
namespace gui {

extern int g_selectedProfileLocation;

class UserProfilesPage : public mw::gui::Page {
public:
	void pageWillAppear();

	void showProfile();

	void toggleAutoRecall();
	void toggleIsAutoRecallLocation();
	void recall();
	void save();
	void deleteProfile();
	void editRemark();

	profile::Parameters profile;

private:
	static void onSaveFinish(char *remark = 0, void (*callback)() = 0);
	static void onSaveEditRemarkOk(char *remark);
	static void onSaveYes();
	static void onDeleteProfileYes();
	static void onEditRemarkOk(char *newRemark);
};

}
}
} // namespace eez::app::gui
