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
#include "gui_internal.h"
#include "gui_adv_lripple.h"

namespace eez {
namespace psu {
namespace gui {
namespace adv_lripple {

static int origStatus;
static int status;

static int origAutoMode;
static int autoMode;

void show() {
	origStatus = status = g_channel->isLowRippleEnabled();
	origAutoMode = autoMode = g_channel->isLowRippleAutoEnabled();

	showPage(PAGE_ID_CH_SETTINGS_ADV_LRIPPLE);
}

int getStatus() {
	return status;
}

void toggleStatus() {
	status = status ? 0 : 1;
}

int getAutoMode() {
	return autoMode;
}

void toggleAutoMode() {
	autoMode = autoMode ? 0 : 1;
}

int getDirty() {
	return (origStatus != status || origAutoMode != autoMode) ? 1 : 0;
}

data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_CHANNEL_LRIPPLE_MAX_CURRENT) {
		return data::Value(g_channel->SOA_PREG_CURR, data::VALUE_TYPE_FLOAT_AMPER);
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_MAX_DISSIPATION) {
		return data::Value(g_channel->SOA_POSTREG_PTOT, data::VALUE_TYPE_FLOAT_WATT);
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_STATUS) {
		return snapshot->switches.switch1;
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_AUTO_MODE) {
		return snapshot->switches.switch2;
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_DIRTY) {
		return snapshot->switches.switch3;
	}

	return data::Value();
}

void set() {
	if (!getDirty()) {
		return;
	}

	if (!g_channel->lowRippleEnable(status ? true : false)) {
		errorMessageP(PSTR("Failed to change LRipple status!"), showPreviousPage);
		return;
	}
	
	g_channel->lowRippleAutoEnable(autoMode ? true : false);

	profile::save();
	
	infoMessageP(PSTR("LRipple params changed!"), showPreviousPage);
}

void discard() {
	showPreviousPage();
}

}
}
}
} // namespace eez::psu::gui::adv_lripple
