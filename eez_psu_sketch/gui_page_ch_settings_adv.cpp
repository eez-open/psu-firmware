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

#include "profile.h"

#include "gui_data_snapshot.h"
#include "gui_page_ch_settings_adv.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

data::Value ChSettingsAdvPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_CHANNEL_RPROG_INSTALLED) {
		return data::Value(g_channel->getFeatures() & CH_FEATURE_RPROG ? 1 : 0);
	}

	return data::Value();
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsAdvLRipplePage::ChSettingsAdvLRipplePage() {
	origStatus = status = g_channel->isLowRippleEnabled();
	origAutoMode = autoMode = g_channel->isLowRippleAutoEnabled();
}

void ChSettingsAdvLRipplePage::takeSnapshot(data::Snapshot *snapshot) {
	SetPage::takeSnapshot(snapshot);

	snapshot->flags.switch1 = status;
	snapshot->flags.switch2 = autoMode;
}

data::Value ChSettingsAdvLRipplePage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	data::Value value = SetPage::getData(cursor, id, snapshot);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_MAX_CURRENT) {
		return data::Value(g_channel->SOA_PREG_CURR, data::VALUE_TYPE_FLOAT_AMPER);
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_MAX_DISSIPATION) {
		return data::Value(g_channel->SOA_POSTREG_PTOT, data::VALUE_TYPE_FLOAT_WATT);
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_STATUS) {
		return snapshot->flags.switch1;
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_AUTO_MODE) {
		return snapshot->flags.switch2;
	}

	return data::Value();
}

void ChSettingsAdvLRipplePage::toggleStatus() {
	status = status ? 0 : 1;
}

void ChSettingsAdvLRipplePage::toggleAutoMode() {
	autoMode = autoMode ? 0 : 1;
}

int ChSettingsAdvLRipplePage::getDirty() {
	return (origStatus != status || origAutoMode != autoMode) ? 1 : 0;
}

void ChSettingsAdvLRipplePage::set() {
	if (getDirty()) {
		if (!g_channel->lowRippleEnable(status ? true : false)) {
			errorMessageP(PSTR("Failed to change LRipple status!"));
			return;
		}
	
		g_channel->lowRippleAutoEnable(autoMode ? true : false);

		profile::save();
	
		infoMessageP(PSTR("LRipple params changed!"), actions[ACTION_ID_SHOW_CH_SETTINGS_ADV]);
	}
}

////////////////////////////////////////////////////////////////////////////////

void ChSettingsAdvRSensePage::takeSnapshot(data::Snapshot *snapshot) {
	snapshot->flags.switch1 = g_channel->isRemoteSensingEnabled() ? 1 : 0;
}

data::Value ChSettingsAdvRSensePage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_CHANNEL_RSENSE_STATUS) {
		return snapshot->flags.switch1;
	}
	return data::Value();
}

void ChSettingsAdvRSensePage::toggleStatus() {
	g_channel->remoteSensingEnable(!g_channel->isRemoteSensingEnabled());
}

////////////////////////////////////////////////////////////////////////////////

void ChSettingsAdvRProgPage::takeSnapshot(data::Snapshot *snapshot) {
	snapshot->flags.switch1 = g_channel->isRemoteProgrammingEnabled() ? 1 : 0;
}

data::Value ChSettingsAdvRProgPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_CHANNEL_RPROG_STATUS) {
		return snapshot->flags.switch1;
	}
	return data::Value();
}

void ChSettingsAdvRProgPage::toggleStatus() {
	g_channel->remoteProgrammingEnable(!g_channel->isRemoteProgrammingEnabled());
}

////////////////////////////////////////////////////////////////////////////////

}
}
} // namespace eez::psu::gui
