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

#include "gui_data_snapshot.h"
#include "gui_keypad.h"
#include "gui_page_user_profiles.h"

namespace eez {
namespace psu {
namespace gui {

static int g_selectedProfileLocation;

void UserProfilesPage::pageWillAppear() {
	if (getActivePageId() == PAGE_ID_USER_PROFILE_0_SETTINGS || getActivePageId() == PAGE_ID_USER_PROFILE_SETTINGS) {
		profile::load(g_selectedProfileLocation, &profile);
	} else {
		g_selectedProfileLocation = -1;
	}
}

void UserProfilesPage::takeSnapshot(data::Snapshot *snapshot) {
	snapshot->flags.switch1 = persist_conf::isProfileAutoRecallEnabled();
	if (g_selectedProfileLocation != -1) {
		snapshot->profile.status = profile::isValid(g_selectedProfileLocation) ? 1 : 0;
		snapshot->profile.isAutoRecallLocation = persist_conf::getProfileAutoRecallLocation() == g_selectedProfileLocation ? 1 : 0;
        profile::getName(g_selectedProfileLocation, snapshot->profile.remark, sizeof(snapshot->profile.remark));
		
		for (int i = 0; i < CH_MAX; ++i) {
			snapshot->profile.channels[i].outputStatus = profile.channels[i].flags.output_enabled;
			snapshot->profile.channels[i].u_set = profile.channels[i].u_set;
			snapshot->profile.channels[i].i_set = profile.channels[i].i_set;
		}
	} else {
		snapshot->profile.isAutoRecallLocation = 0;
	}
}

data::Value UserProfilesPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_PROFILES_AUTO_RECALL_STATUS) {
		return data::Value(snapshot->flags.switch1);
	}

	if (id == DATA_ID_PROFILES_AUTO_RECALL_LOCATION) {
		return data::Value(persist_conf::getProfileAutoRecallLocation());
	}

	if (g_selectedProfileLocation != -1) {
		if (id == DATA_ID_PROFILE_STATUS) {
			return data::Value(snapshot->profile.status);
		}

		if (id == DATA_ID_PROFILE_LABEL) {
			return data::Value(g_selectedProfileLocation, data::VALUE_TYPE_USER_PROFILE_LABEL);
		}

		if (id == DATA_ID_PROFILE_REMARK) {
			return data::Value(snapshot->profile.remark);
		}

		if (id == DATA_ID_PROFILE_IS_AUTO_RECALL_LOCATION) {
			return data::Value(snapshot->profile.isAutoRecallLocation);
		}

		if (cursor.i >= 0 && cursor.i < CH_MAX) {
			if (id == DATA_ID_PROFILE_CHANNEL_OUTPUT_STATE) {
				return data::Value(snapshot->profile.channels[cursor.i].outputStatus);
			}

			if (id == DATA_ID_PROFILE_CHANNEL_U_SET) {
				return data::Value(snapshot->profile.channels[cursor.i].u_set, data::VALUE_TYPE_FLOAT_VOLT);
			}

			if (id == DATA_ID_PROFILE_CHANNEL_I_SET) {
				return data::Value(snapshot->profile.channels[cursor.i].i_set, data::VALUE_TYPE_FLOAT_AMPER);
			}
		}
	} else if (cursor.i >= 0) {
		if (id == DATA_ID_PROFILE_LABEL) {
			return data::Value(cursor.i, data::VALUE_TYPE_USER_PROFILE_LABEL);
		}

		if (id == DATA_ID_PROFILE_REMARK) {
			return data::Value(cursor.i, data::VALUE_TYPE_USER_PROFILE_REMARK);
		}
	}

	return data::Value();
}

void UserProfilesPage::showProfile() {
	g_selectedProfileLocation = g_foundWidgetAtDown.cursor.i;
	pushPage(g_selectedProfileLocation == 0 ? PAGE_ID_USER_PROFILE_0_SETTINGS : PAGE_ID_USER_PROFILE_SETTINGS);
}

void UserProfilesPage::toggleAutoRecall() {
	bool enable = persist_conf::isProfileAutoRecallEnabled() ? false : true;
	if (!persist_conf::enableProfileAutoRecall(enable)) {
		errorMessageP(PSTR("Failed!"));
	}
}

void UserProfilesPage::toggleIsAutoRecallLocation() {
	if (profile::isValid(g_selectedProfileLocation)) {
		if (persist_conf::setProfileAutoRecallLocation(g_selectedProfileLocation)) {
			profile::load(g_selectedProfileLocation, &profile);
		} else {
			errorMessageP(PSTR("Failed!"));
		}
	}
}

void UserProfilesPage::recall() {
	if (g_selectedProfileLocation > 0 && profile::isValid(g_selectedProfileLocation)) {
	    if (profile::recall(g_selectedProfileLocation)) {
			infoMessageP(PSTR("Profile parameters loaded"));
		} else {
			errorMessageP(PSTR("Failed!"));
		}
	}
}

void UserProfilesPage::save() {
    if (g_selectedProfileLocation > 0) {
		if (profile::saveAtLocation(g_selectedProfileLocation)) {
			infoMessageP(PSTR("Current parameters saved"));
		} else {
			errorMessageP(PSTR("Failed!"));
		}
    }
}

void UserProfilesPage::onDeleteProfileYes() {
	popPage();

	if (g_selectedProfileLocation > 0 && profile::isValid(g_selectedProfileLocation)) {
		if (profile::deleteLocation(g_selectedProfileLocation)) {
			infoMessageP(PSTR("Profile deleted"));
		} else {
			errorMessageP(PSTR("Failed!"));
		}
	}
}

void UserProfilesPage::deleteProfile() {
	if (g_selectedProfileLocation > 0 && profile::isValid(g_selectedProfileLocation)) {
		yesNoDialog(PAGE_ID_YES_NO, "Are you sure?", onDeleteProfileYes, 0, 0);
	}
}

void UserProfilesPage::onEditRemarkOk(char *newRemark) {
	if (profile::setName(g_selectedProfileLocation, newRemark, strlen(newRemark))) {
		infoMessageP(PSTR("Remark changed"), popPage);
	} else {
		errorMessageP(PSTR("Failed!"));
	}
}

void UserProfilesPage::editRemark() {
	if (g_selectedProfileLocation > 0 && profile::isValid(g_selectedProfileLocation)) {
		char remark[PROFILE_NAME_MAX_LENGTH + 1];
        profile::getName(g_selectedProfileLocation, remark, sizeof(remark));
		keypad::startPush(0, remark, PROFILE_NAME_MAX_LENGTH, false, onEditRemarkOk, 0);
	}
}

}
}
} // namespace eez::psu::gui
