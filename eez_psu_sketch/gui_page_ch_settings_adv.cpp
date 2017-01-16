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
#include "channel_dispatcher.h"

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
	origAutoMode = autoMode = g_channel->isLowRippleAutoEnabled();
	origStatus = status = g_channel->isLowRippleEnabled();
}

void ChSettingsAdvLRipplePage::takeSnapshot(data::Snapshot *snapshot) {
	SetPage::takeSnapshot(snapshot);

    bool isAllowed = g_channel->isLowRippleAllowed(micros());

	snapshot->flags.switch1 = autoMode;
	snapshot->flags.switch2 = g_channel->isLowRippleAllowed(micros());
    snapshot->flags.switch3 = status;

    if (!isAllowed) {
        origStatus = status = false;
    }
}

data::Value ChSettingsAdvLRipplePage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	data::Value value = SetPage::getData(cursor, id, snapshot);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_MAX_DISSIPATION) {
		return data::Value(g_channel->SOA_POSTREG_PTOT, data::VALUE_TYPE_FLOAT_WATT);
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_CALCULATED_DISSIPATION) {
        data::ChannelSnapshot &channelSnapshot = snapshot->channelSnapshots[g_channel->index - 1];
		return data::Value(channelSnapshot.i_mon * (g_channel->SOA_VIN - channelSnapshot.u_mon), data::VALUE_TYPE_FLOAT_WATT);
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_AUTO_MODE) {
		return snapshot->flags.switch1;
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_IS_ALLOWED) {
		return snapshot->flags.switch2;
	}

    if (id == DATA_ID_CHANNEL_LRIPPLE_STATUS) {
		return snapshot->flags.switch3;
	}

	return data::Value();
}

void ChSettingsAdvLRipplePage::toggleAutoMode() {
	autoMode = autoMode ? 0 : 1;
}

void ChSettingsAdvLRipplePage::toggleStatus() {
	status = status ? 0 : 1;
}

int ChSettingsAdvLRipplePage::getDirty() {
	return (origAutoMode != autoMode || origStatus != status) ? 1 : 0;
}

void ChSettingsAdvLRipplePage::set() {
	if (getDirty()) {
		if (!channel_dispatcher::lowRippleEnable(*g_channel, status ? true : false)) {
			errorMessageP(PSTR("Failed to change LRipple status!"));
			return;
		}
	
		channel_dispatcher::lowRippleAutoEnable(*g_channel, autoMode ? true : false);

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
	channel_dispatcher::remoteSensingEnable(*g_channel, !g_channel->isRemoteSensingEnabled());
}

////////////////////////////////////////////////////////////////////////////////

void ChSettingsAdvRProgPage::toggleStatus() {
	g_channel->remoteProgrammingEnable(!g_channel->isRemoteProgrammingEnabled());
}

////////////////////////////////////////////////////////////////////////////////

int ChSettingsAdvCouplingPage::selectedMode = 0;

data::Value ChSettingsAdvCouplingPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_CHANNEL_COUPLING_SELECTED_MODE) {
		return data::Value(selectedMode);
	}

	return data::Value();
}

void ChSettingsAdvCouplingPage::uncouple() {
    channel_dispatcher::setType(channel_dispatcher::TYPE_NONE);
	infoMessageP(PSTR("Channels uncoupled!"));
}

void ChSettingsAdvCouplingPage::setParallelInfo() {
    selectedMode = 0;
    pushPage(PAGE_ID_CH_SETTINGS_ADV_COUPLING_INFO);
}

void ChSettingsAdvCouplingPage::setSeriesInfo() {
    selectedMode = 1;
    pushPage(PAGE_ID_CH_SETTINGS_ADV_COUPLING_INFO);
}

void ChSettingsAdvCouplingPage::setParallel() {
    if (selectedMode == 0) {
        if (channel_dispatcher::setType(channel_dispatcher::TYPE_PARALLEL)) {
	        infoMessageP(PSTR("Channels coupled in parallel!"), popPage);
        }
    }
}

void ChSettingsAdvCouplingPage::setSeries() {
    if (selectedMode == 1) {
        if (channel_dispatcher::setType(channel_dispatcher::TYPE_SERIES)) {
    	    infoMessageP(PSTR("Channels coupled in series!"), popPage);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ChSettingsAdvTrackingPage::toggleTrackingMode() {
    if (channel_dispatcher::isTracked()) {
        channel_dispatcher::setType(channel_dispatcher::TYPE_NONE);
        infoMessageP(PSTR("Tracking disabled!"));
    } else {
        channel_dispatcher::setType(channel_dispatcher::TYPE_TRACKED);
	    infoMessageP(PSTR("Tracking enabled!"));
    }
}

}
}
} // namespace eez::psu::gui
