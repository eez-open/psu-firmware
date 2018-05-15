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

#include "eez/psu/psu.h"

#if OPTION_DISPLAY

#include "eez/psu/gui/keypad.h"
#include "eez/psu/gui/page_user_profiles.h"
#include "eez/psu/channel_dispatcher.h"

#include "eez/psu/gui/psu.h"
#include "eez/app/gui/document.h"

using namespace eez::app::gui;

namespace eez {
namespace psu {
namespace gui {

int g_selectedProfileLocation;

void UserProfilesPage::pageWillAppear() {
	if (getActivePageId() == PAGE_ID_USER_PROFILE_0_SETTINGS || getActivePageId() == PAGE_ID_USER_PROFILE_SETTINGS) {
		profile::load(g_selectedProfileLocation, &profile);
	} else {
		g_selectedProfileLocation = -1;
	}
}

void UserProfilesPage::showProfile() {
	g_selectedProfileLocation = g_foundWidgetAtDown.cursor.i;
	pushPage(g_selectedProfileLocation == 0 ? PAGE_ID_USER_PROFILE_0_SETTINGS : PAGE_ID_USER_PROFILE_SETTINGS);
}

void UserProfilesPage::toggleAutoRecall() {
	bool enable = persist_conf::isProfileAutoRecallEnabled() ? false : true;
	if (!persist_conf::enableProfileAutoRecall(enable)) {
		errorMessageP("Failed!");
	}
}

void UserProfilesPage::toggleIsAutoRecallLocation() {
	if (profile::isValid(g_selectedProfileLocation)) {
		if (persist_conf::setProfileAutoRecallLocation(g_selectedProfileLocation)) {
			profile::load(g_selectedProfileLocation, &profile);
		} else {
			errorMessageP("Failed!");
		}
	}
}

void UserProfilesPage::recall() {
	if (g_selectedProfileLocation > 0 && profile::isValid(g_selectedProfileLocation)) {
	    if (profile::recall(g_selectedProfileLocation)) {
			infoMessageP("Profile parameters loaded");
		} else {
			errorMessageP("Failed!");
		}
	}
}

void UserProfilesPage::onSaveFinish(char *remark, void (*callback)()) {
	if (profile::saveAtLocation(g_selectedProfileLocation, remark)) {
		infoMessageP("Current parameters saved", callback);
	} else {
		errorMessageP("EEPROM save failed!");
	}
}

void UserProfilesPage::onSaveEditRemarkOk(char *remark) {
	onSaveFinish(remark, popPage);
}

void UserProfilesPage::onSaveYes() {
	if (g_selectedProfileLocation > 0) {
		char remark[PROFILE_NAME_MAX_LENGTH + 1];
		profile::getSaveName(&(((UserProfilesPage *)getActivePage())->profile), remark);

		Keypad::startPush(0, remark, PROFILE_NAME_MAX_LENGTH, false, onSaveEditRemarkOk, 0);
	} else {
		onSaveFinish();
	}
}

void UserProfilesPage::save() {
    if (g_selectedProfileLocation > 0) {
		if (profile::isValid(g_selectedProfileLocation)) {
			areYouSure(onSaveYes);
		} else {
			onSaveYes();
		}
    }
}

void UserProfilesPage::onDeleteProfileYes() {
	if (g_selectedProfileLocation > 0 && profile::isValid(g_selectedProfileLocation)) {
		if (profile::deleteLocation(g_selectedProfileLocation)) {
			infoMessageP("Profile deleted");
		} else {
			errorMessageP("Failed!");
		}
	}
}

void UserProfilesPage::deleteProfile() {
	if (g_selectedProfileLocation > 0 && profile::isValid(g_selectedProfileLocation)) {
		areYouSure(onDeleteProfileYes);
	}
}

void UserProfilesPage::onEditRemarkOk(char *newRemark) {
	if (profile::setName(g_selectedProfileLocation, newRemark, strlen(newRemark))) {
		infoMessageP("Remark changed", popPage);
	} else {
		errorMessageP("Failed!", popPage);
	}
}

void UserProfilesPage::editRemark() {
	if (g_selectedProfileLocation > 0 && profile::isValid(g_selectedProfileLocation)) {
		char remark[PROFILE_NAME_MAX_LENGTH + 1];
        profile::getName(g_selectedProfileLocation, remark, sizeof(remark));
		Keypad::startPush(0, remark, PROFILE_NAME_MAX_LENGTH, false, onEditRemarkOk, 0);
	}
}

}
}
} // namespace eez::psu::gui

#endif