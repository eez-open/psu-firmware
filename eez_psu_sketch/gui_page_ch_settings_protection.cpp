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

#include "gui_data_snapshot.h"
#include "gui_page_ch_settings_protection.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

void ChSettingsProtectionPage::clear() {
	g_channel->clearProtection();

	infoMessageP(PSTR("Cleared!"), actions[ACTION_ID_SHOW_CH_SETTINGS_PROT]);
}

void onClearAndDisableYes() {
	g_channel->clearProtection();
	g_channel->disableProtection();
	profile::save();

	infoMessageP(PSTR("Cleared and disabled!"), actions[ACTION_ID_SHOW_CH_SETTINGS_PROT]);
}

void ChSettingsProtectionPage::clearAndDisable() {
	areYouSure(onClearAndDisableYes);
}

data::Value ChSettingsProtectionPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_CHANNEL_PROTECTION_OTP_INSTALLED) {
		return temperature::isChannelSensorInstalled(g_channel);
	}
	return data::Value();
}

////////////////////////////////////////////////////////////////////////////////

void ChSettingsProtectionSetPage::takeSnapshot(data::Snapshot *snapshot) {
	SetPage::takeSnapshot(snapshot);

	snapshot->flags.switch1 = state;

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
	channelSnapshot.flags.tempStatus = 2;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
	data::ChannelSnapshot &channelSnapshot = snapshot->channelSnapshots[g_channel->index - 1];

	temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + g_channel->index - 1];
	if (tempSensor.isInstalled()) {
		if (tempSensor.isTestOK()) {
			channelSnapshot.flags.temperatureStatus = 1;
			channelSnapshot.temperature = tempSensor.temperature;
		} else {
			channelSnapshot.flags.temperatureStatus = 0;
		}
	} else {
		channelSnapshot.flags.temperatureStatus = 2;
	}
#endif
}

data::Value ChSettingsProtectionSetPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	data::Value value = SetPage::getData(cursor, id, snapshot);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

	data::ChannelSnapshot &channelSnapshot = snapshot->channelSnapshots[g_channel->index - 1];

	if (id == DATA_ID_CHANNEL_PROTECTION_OVP_LIMIT ||
		id == DATA_ID_CHANNEL_PROTECTION_OCP_LIMIT ||
		id == DATA_ID_CHANNEL_PROTECTION_OPP_LIMIT) {
		return limit;
	}

	if (id == DATA_ID_CHANNEL_PROTECTION_OVP_STATE ||
		id == DATA_ID_CHANNEL_PROTECTION_OCP_STATE ||
		id == DATA_ID_CHANNEL_PROTECTION_OPP_STATE ||
		id == DATA_ID_CHANNEL_PROTECTION_OTP_STATE) {
		return snapshot->flags.switch1;
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

	if (id == DATA_ID_CHANNEL_TEMP_STATUS) {
		return data::Value(channelSnapshot.flags.temperatureStatus);
	}

	if (id == DATA_ID_CHANNEL_TEMP && channelSnapshot.flags.temperatureStatus == 1) {
		return data::Value(channelSnapshot.temperature, data::VALUE_TYPE_FLOAT_CELSIUS);
	}

	return data::Value();
}

int ChSettingsProtectionSetPage::getDirty() {
	return (origState != state || origLimit != limit || origLevel != level || origDelay != delay) ? 1 : 0;
}

void ChSettingsProtectionSetPage::set() {
	if (getDirty()) {
		setParams();
		profile::save();
		infoMessageP(PSTR("Protection params changed!"), actions[ACTION_ID_SHOW_CH_SETTINGS_PROT]);
	}
}

void ChSettingsProtectionSetPage::toggleState() {
	state = state ? 0 : 1;
}

void ChSettingsProtectionSetPage::onLimitSet(float value) {
	popPage();
	ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getActivePage();
	page->limit = data::Value(value, page->limit.getType());
}

void ChSettingsProtectionSetPage::editLimit() {
	numeric_keypad::Options options;

	options.editUnit = limit.getType();

	options.min = minLimit;
	options.max = maxLimit;
	options.def = defLimit;

	options.flags.genericNumberKeypad = true;
	options.flags.maxButtonEnabled = true;
	options.flags.defButtonEnabled = true;
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	numeric_keypad::start(0, options, onLimitSet);
}

void ChSettingsProtectionSetPage::onLevelSet(float value) {
	popPage();
	ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getActivePage();
	page->level = data::Value(value, page->level.getType());
}

void ChSettingsProtectionSetPage::editLevel() {
	numeric_keypad::Options options;

	options.editUnit = level.getType();

	options.min = minLevel;
	options.max = maxLevel;
	options.def = defLevel;

	options.flags.genericNumberKeypad = true;
	options.flags.maxButtonEnabled = true;
	options.flags.defButtonEnabled = true;
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	numeric_keypad::start(0, options, onLevelSet);
}

void ChSettingsProtectionSetPage::onDelaySet(float value) {
	popPage();
	ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getActivePage();
	page->delay = data::Value(value, page->delay.getType());
}

void ChSettingsProtectionSetPage::editDelay() {
	numeric_keypad::Options options;

	options.editUnit = delay.getType();

	options.min = minDelay;
	options.max = maxDelay;
	options.def = defaultDelay;

	options.flags.genericNumberKeypad = true;
	options.flags.maxButtonEnabled = true;
	options.flags.defButtonEnabled = true;
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	numeric_keypad::start(0, options, onDelaySet);
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsOvpProtectionPage::ChSettingsOvpProtectionPage() {
	origState = state = g_channel->prot_conf.flags.u_state ? 1 : 0;

	origLimit = limit = data::Value(g_channel->u.limit, data::VALUE_TYPE_FLOAT_VOLT);
	minLimit = g_channel->U_MIN;
	maxLimit = g_channel->U_MAX;
	defLimit = g_channel->U_MAX;

	origLevel = level = data::Value(g_channel->prot_conf.u_level, data::VALUE_TYPE_FLOAT_VOLT);
	minLevel = g_channel->u.set;
	maxLevel = g_channel->U_MAX;
	defLevel = g_channel->U_MAX;

	origDelay = delay = data::Value(g_channel->prot_conf.u_delay, data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OVP_MIN_DELAY;
	maxDelay = g_channel->OVP_MAX_DELAY;
	defaultDelay = g_channel->OVP_DEFAULT_DELAY;
}

void ChSettingsOvpProtectionPage::setParams() {
	g_channel->setVoltageLimit(limit.getFloat());
	g_channel->prot_conf.flags.u_state = state;
	g_channel->prot_conf.u_level = level.getFloat();
	g_channel->prot_conf.u_delay = delay.getFloat();
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsOcpProtectionPage::ChSettingsOcpProtectionPage() {
	origState = state = g_channel->prot_conf.flags.i_state ? 1 : 0;

	origLimit = limit = data::Value(g_channel->i.limit, data::VALUE_TYPE_FLOAT_AMPER);
	minLimit = g_channel->I_MIN;
	maxLimit = g_channel->getCurrentMaxLimit();
	defLimit = g_channel->I_MAX;

	origLevel = level = 0;

	origDelay = delay = data::Value(g_channel->prot_conf.i_delay, data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OCP_MIN_DELAY;
	maxDelay = g_channel->OCP_MAX_DELAY;
	defaultDelay = g_channel->OCP_DEFAULT_DELAY;
}

void ChSettingsOcpProtectionPage::setParams() {
	g_channel->setCurrentLimit(limit.getFloat());
	g_channel->prot_conf.flags.i_state = state;
	g_channel->prot_conf.i_delay = delay.getFloat();
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsOppProtectionPage::ChSettingsOppProtectionPage() {
	origState = state = g_channel->prot_conf.flags.p_state ? 1 : 0;

	origLimit = limit = data::Value(g_channel->p_limit, data::VALUE_TYPE_FLOAT_WATT);
	minLimit = g_channel->U_MIN * g_channel->I_MIN;
	maxLimit = g_channel->U_MAX * g_channel->I_MAX;
	defLimit = g_channel->PTOT;

	origLevel = level = data::Value(g_channel->prot_conf.p_level, data::VALUE_TYPE_FLOAT_WATT);
	minLevel = g_channel->OPP_MIN_LEVEL;
	maxLevel = g_channel->OPP_MAX_LEVEL;
	defLevel = g_channel->OPP_DEFAULT_LEVEL;

	origDelay = delay = data::Value(g_channel->prot_conf.p_delay, data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OPP_MIN_DELAY;
	maxDelay = g_channel->OPP_MAX_DELAY;
	defaultDelay = g_channel->OPP_DEFAULT_DELAY;
}

void ChSettingsOppProtectionPage::setParams() {
	g_channel->setPowerLimit(limit.getFloat());
	g_channel->prot_conf.flags.p_state = state;
	g_channel->prot_conf.p_level = level.getFloat();
	g_channel->prot_conf.p_delay = delay.getFloat();
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsOtpProtectionPage::ChSettingsOtpProtectionPage() {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
	origState = state = temperature::getChannelSensorState(g_channel) ? 1 : 0;

	origLimit = limit = 0;
	minLimit = 0;
	maxLimit = 0;
	defLimit = 0;

	origLevel = level = data::Value(temperature::getChannelSensorLevel(g_channel), data::VALUE_TYPE_FLOAT_CELSIUS);
	minLevel = OTP_MAIN_MIN_LEVEL;
	maxLevel = OTP_MAIN_MAX_LEVEL;
	defLevel = OTP_MAIN_DEFAULT_LEVEL;

	origDelay = delay = data::Value(temperature::getChannelSensorDelay(g_channel), data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = OTP_MAIN_MIN_DELAY;
	maxDelay = OTP_MAIN_MAX_DELAY;
	defaultDelay = OTP_MAIN_DEFAULT_DELAY;
#endif
}

void ChSettingsOtpProtectionPage::setParams() {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4	
	temperature::setChannelSensorState(g_channel, state ? true : false);
	temperature::setChannelSensorLevel(g_channel, level.getFloat());
	temperature::setChannelSensorDelay(g_channel, delay.getFloat());
#endif
}

}
}
} // namespace eez::psu::gui
