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
#include "temperature.h"

#include "gui_internal.h"
#include "gui_protection.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {
namespace protection {

static int editPage;

static int state;

static data::Value level;
static float minLevel;
static float maxLevel;
static float defLevel;

static data::Value delay;
static float minDelay;
static float maxDelay;
static float defaultDelay;

void clear() {
	g_channel->clearProtection();

	infoMessageP(PSTR("Cleared!"), 0);
}

void clearAndDisable() {
	g_channel->clearProtection();
	g_channel->disableProtection();
	profile::save();

	infoMessageP(PSTR("Cleared and disabled!"), 0);
}

void editOVP() {
	state = g_channel->prot_conf.flags.u_state ? 1 : 0;

	level = data::Value(g_channel->prot_conf.u_level, data::VALUE_TYPE_FLOAT_VOLT);
	minLevel = g_channel->u.set;
	maxLevel = g_channel->U_MAX;
	defLevel = g_channel->U_MAX;

	delay = data::Value(g_channel->prot_conf.u_delay, data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OVP_MIN_DELAY;
	maxDelay = g_channel->OVP_MAX_DELAY;
	defaultDelay = g_channel->OVP_DEFAULT_DELAY;

	editPage = PAGE_ID_CH_SETTINGS_PROT_OVP;
	showPage(editPage);
}

void editOCP() {
	state = g_channel->prot_conf.flags.i_state ? 1 : 0;

	delay = data::Value(g_channel->prot_conf.i_delay, data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OCP_MIN_DELAY;
	maxDelay = g_channel->OCP_MAX_DELAY;
	defaultDelay = g_channel->OCP_DEFAULT_DELAY;

	editPage = PAGE_ID_CH_SETTINGS_PROT_OCP;
	showPage(editPage);
}

void editOPP() {
	state = g_channel->prot_conf.flags.p_state ? 1 : 0;

	level = data::Value(g_channel->prot_conf.p_level, data::VALUE_TYPE_FLOAT_WATT);
	minLevel = g_channel->OPP_MIN_LEVEL;
	maxLevel = g_channel->OPP_MAX_LEVEL;
	defLevel = g_channel->OPP_DEFAULT_LEVEL;

	delay = data::Value(g_channel->prot_conf.p_delay, data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OPP_MIN_DELAY;
	maxDelay = g_channel->OPP_MAX_DELAY;
	defaultDelay = g_channel->OPP_DEFAULT_DELAY;

	editPage = PAGE_ID_CH_SETTINGS_PROT_OPP;
	showPage(editPage);
}

void editOTP() {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
	state = temperature::getChannelSensorState(g_channel) ? 1 : 0;

	level = data::Value(temperature::getChannelSensorLevel(g_channel), data::VALUE_TYPE_FLOAT_CELSIUS);
	minLevel = OTP_MAIN_MIN_LEVEL;
	maxLevel = OTP_MAIN_MAX_LEVEL;
	defLevel = OTP_MAIN_DEFAULT_LEVEL;

	delay = data::Value(temperature::getChannelSensorDelay(g_channel), data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = OTP_MAIN_MIN_DELAY;
	maxDelay = OTP_MAIN_MAX_DELAY;
	defaultDelay = OTP_MAIN_DEFAULT_DELAY;

	editPage = PAGE_ID_CH_SETTINGS_PROT_OTP;
	showPage(editPage);
#endif
}

data::Value getData(const data::Cursor &cursor, uint8_t id) {
	if (id == DATA_ID_CHANNEL_PROTECTION_OTP_INSTALLED) {
		return temperature::isChannelSensorInstalled(g_channel);
	}

	if (id == DATA_ID_CHANNEL_PROTECTION_OVP_STATE ||
		id == DATA_ID_CHANNEL_PROTECTION_OCP_STATE ||
		id == DATA_ID_CHANNEL_PROTECTION_OPP_STATE ||
		id == DATA_ID_CHANNEL_PROTECTION_OTP_STATE) {
		return state;
	}

	if (id == DATA_ID_CHANNEL_PROTECTION_OVP_LEVEL ||
		id == DATA_ID_CHANNEL_PROTECTION_OPP_LEVEL ||
		id == DATA_ID_CHANNEL_PROTECTION_OTP_LEVEL) {
		return level;
	}

	if (id == DATA_ID_CHANNEL_PROTECTION_OVP_DELAY ||
		id == DATA_ID_CHANNEL_PROTECTION_OCP_DELAY ||
		id == DATA_ID_CHANNEL_PROTECTION_OPP_DELAY ||
		id == DATA_ID_CHANNEL_PROTECTION_OTP_DELAY) {
		return delay;
	}

	return data::Value();
}

void toggleState() {
	state = state ? 0 : 1;
	refreshPage();
}

void onLevelSet(float value) {
	level = data::Value(value, level.getType());
	showPreviousPage();
}

void editLevel() {
	numeric_keypad::start(0, level.getType(), minLevel, maxLevel, true, defLevel, true, onLevelSet, showPreviousPage, true);
}

void onDelaySet(float value) {
	delay = data::Value(value, delay.getType());
	showPreviousPage();
}

void editDelay() {
	numeric_keypad::start(0, delay.getType(), minDelay, maxDelay, true, defaultDelay, true, onDelaySet, showPreviousPage, true);
}

void set() {
	if (editPage == PAGE_ID_CH_SETTINGS_PROT_OVP) {
		g_channel->prot_conf.flags.u_state = state;
		g_channel->prot_conf.u_level = level.getFloat();
		g_channel->prot_conf.u_delay = delay.getFloat();
	} else if (editPage == PAGE_ID_CH_SETTINGS_PROT_OCP) {
		g_channel->prot_conf.flags.i_state = state;
		g_channel->prot_conf.i_delay = delay.getFloat();
	} else if (editPage == PAGE_ID_CH_SETTINGS_PROT_OPP) {
		g_channel->prot_conf.flags.p_state = state;
		g_channel->prot_conf.p_level = level.getFloat();
		g_channel->prot_conf.p_delay = delay.getFloat();
	} 
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4	
	else if (editPage == PAGE_ID_CH_SETTINGS_PROT_OTP) {
		temperature::setChannelSensorState(g_channel, state ? true : false);
		temperature::setChannelSensorLevel(g_channel, level.getFloat());
		temperature::setChannelSensorDelay(g_channel, delay.getFloat());
	}
#endif
	
	profile::save();
	
	infoMessageP(PSTR("Protection params changed!"), showPreviousPage);
}

void discard() {
	showPreviousPage();
}

}
}
}
} // namespace eez::psu::gui::protection
