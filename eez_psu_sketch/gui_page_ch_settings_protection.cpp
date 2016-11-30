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
#include "channel_coupling.h"

#include "gui_data_snapshot.h"
#include "gui_page_ch_settings_protection.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

void ChSettingsProtectionPage::clear() {
	channel_coupling::clearProtection(*g_channel);

	infoMessageP(PSTR("Cleared!"), actions[ACTION_ID_SHOW_CH_SETTINGS_PROT]);
}

void onClearAndDisableYes() {
	channel_coupling::clearProtection(*g_channel);
	channel_coupling::disableProtection(*g_channel);
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

	data::ChannelSnapshot &channelSnapshot = snapshot->channelSnapshots[g_channel->index - 1];

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
	channelSnapshot.flags.temperatureStatus = 2;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
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

    if (id == DATA_ID_CHANNEL_PROTECTION_OCP_MAX_CURRENT_LIMIT_CAUSE) {
        return data::Value(g_channel->getMaxCurrentLimitCause());
    }

	return data::Value();
}

int ChSettingsProtectionSetPage::getDirty() {
	return (origState != state || origLimit != limit || origLevel != level || origDelay != delay) ? 1 : 0;
}

void ChSettingsProtectionSetPage::onSetFinish(bool showInfo) {
	profile::save();
	if (showInfo) {
		infoMessageP(PSTR("Protection params changed!"), actions[ACTION_ID_SHOW_CH_SETTINGS_PROT]);
	} else {
		actions[ACTION_ID_SHOW_CH_SETTINGS_PROT]();
	}
}

void ChSettingsProtectionSetPage::set() {
	if (getDirty()) {
		setParams(true);
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

	origLimit = limit = data::Value(channel_coupling::getULimit(*g_channel), data::VALUE_TYPE_FLOAT_VOLT);
	minLimit = channel_coupling::getUMin(*g_channel);
	maxLimit = channel_coupling::getUMax(*g_channel);
	defLimit = channel_coupling::getUMax(*g_channel);

	origLevel = level = data::Value(channel_coupling::getUProtectionLevel(*g_channel), data::VALUE_TYPE_FLOAT_VOLT);
	minLevel = channel_coupling::getUSet(*g_channel);
	maxLevel = channel_coupling::getUMax(*g_channel);
	defLevel = channel_coupling::getUMax(*g_channel);

	origDelay = delay = data::Value(g_channel->prot_conf.u_delay, data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OVP_MIN_DELAY;
	maxDelay = g_channel->OVP_MAX_DELAY;
	defaultDelay = g_channel->OVP_DEFAULT_DELAY;
}

void ChSettingsOvpProtectionPage::onSetParamsOk() {
	((ChSettingsOvpProtectionPage *)getActivePage())->setParams(false);
}

void ChSettingsOvpProtectionPage::setParams(bool checkLoad) {
	if (checkLoad && g_channel->isOutputEnabled() && limit.getFloat() < channel_coupling::getUMon(*g_channel) && util::greaterOrEqual(channel_coupling::getIMon(*g_channel), 0, CHANNEL_VALUE_PRECISION)) {
		areYouSureWithMessage(PSTR("This change will affect current load."), onSetParamsOk);
	} else {
		channel_coupling::setVoltageLimit(*g_channel, limit.getFloat());
        channel_coupling::setOvpParameters(*g_channel, state, level.getFloat(), delay.getFloat());
        onSetFinish(checkLoad);
	}
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsOcpProtectionPage::ChSettingsOcpProtectionPage() {
	origState = state = g_channel->prot_conf.flags.i_state ? 1 : 0;

	origLimit = limit = data::Value(channel_coupling::getILimit(*g_channel), data::VALUE_TYPE_FLOAT_AMPER);
	minLimit = channel_coupling::getIMin(*g_channel);
	maxLimit = channel_coupling::getIMaxLimit(*g_channel);
	defLimit = maxLimit;

	origLevel = level = 0;

	origDelay = delay = data::Value(g_channel->prot_conf.i_delay, data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OCP_MIN_DELAY;
	maxDelay = g_channel->OCP_MAX_DELAY;
	defaultDelay = g_channel->OCP_DEFAULT_DELAY;
}

void ChSettingsOcpProtectionPage::onSetParamsOk() {
	((ChSettingsOcpProtectionPage *)getActivePage())->setParams(false);
}

void ChSettingsOcpProtectionPage::setParams(bool checkLoad) {
	if (checkLoad && g_channel->isOutputEnabled() && limit.getFloat() < channel_coupling::getIMon(*g_channel)) {
		areYouSureWithMessage(PSTR("This change will affect current load."), onSetParamsOk);
	} else {
		channel_coupling::setCurrentLimit(*g_channel, limit.getFloat());
        channel_coupling::setOcpParameters(*g_channel, state, delay.getFloat());
		onSetFinish(checkLoad);
	}
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsOppProtectionPage::ChSettingsOppProtectionPage() {
	origState = state = g_channel->prot_conf.flags.p_state ? 1 : 0;

	origLimit = limit = data::Value(channel_coupling::getPowerLimit(*g_channel), data::VALUE_TYPE_FLOAT_WATT);
	minLimit = channel_coupling::getPowerMinLimit(*g_channel);
	maxLimit = channel_coupling::getPowerMaxLimit(*g_channel);
	defLimit = channel_coupling::getPowerDefaultLimit(*g_channel);

	origLevel = level = data::Value(channel_coupling::getPowerProtectionLevel(*g_channel), data::VALUE_TYPE_FLOAT_WATT);
	minLevel = channel_coupling::getOppMinLevel(*g_channel);
	maxLevel = channel_coupling::getOppMaxLevel(*g_channel);
	defLevel = channel_coupling::getOppDefaultLevel(*g_channel);

	origDelay = delay = data::Value(g_channel->prot_conf.p_delay, data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OPP_MIN_DELAY;
	maxDelay = g_channel->OPP_MAX_DELAY;
	defaultDelay = g_channel->OPP_DEFAULT_DELAY;
}

void ChSettingsOppProtectionPage::onSetParamsOk() {
	((ChSettingsOppProtectionPage *)getActivePage())->setParams(false);
}

void ChSettingsOppProtectionPage::setParams(bool checkLoad) {
	if (checkLoad && g_channel->isOutputEnabled()) {
		float pMon = channel_coupling::getUMon(*g_channel) * channel_coupling::getIMon(*g_channel);
		if (limit.getFloat() < pMon && util::greaterOrEqual(channel_coupling::getIMon(*g_channel), 0, CHANNEL_VALUE_PRECISION)) {
			areYouSureWithMessage(PSTR("This change will affect current load."), onSetParamsOk);
			return;
		}
	}

	channel_coupling::setPowerLimit(*g_channel, limit.getFloat());
    channel_coupling::setOppParameters(*g_channel, state, level.getFloat(), delay.getFloat());
	onSetFinish(checkLoad);
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
	defaultDelay = OTP_CH_DEFAULT_DELAY;
#endif
}

void ChSettingsOtpProtectionPage::setParams(bool checkLoad) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4	
	channel_coupling::setOtpParameters(*g_channel, state, level.getFloat(), delay.getFloat());
	onSetFinish(checkLoad);
#endif
}

}
}
} // namespace eez::psu::gui
