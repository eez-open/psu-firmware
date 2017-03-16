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

#if OPTION_DISPLAY

#include "profile.h"
#include "temperature.h"
#include "channel_dispatcher.h"

#include "gui_page_ch_settings_protection.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

void ChSettingsProtectionPage::clear() {
	channel_dispatcher::clearProtection(*g_channel);

	infoMessageP(PSTR("Cleared!"), actions[ACTION_ID_SHOW_CH_SETTINGS_PROT]);
}

void onClearAndDisableYes() {
	channel_dispatcher::clearProtection(*g_channel);
	channel_dispatcher::disableProtection(*g_channel);
	profile::save();

	infoMessageP(PSTR("Cleared and disabled!"), actions[ACTION_ID_SHOW_CH_SETTINGS_PROT]);
}

void ChSettingsProtectionPage::clearAndDisable() {
	areYouSure(onClearAndDisableYes);
}

data::Value ChSettingsProtectionPage::getData(const data::Cursor &cursor, uint8_t id) {
	if (id == DATA_ID_CHANNEL_PROTECTION_OTP_INSTALLED) {
		return temperature::isChannelSensorInstalled(g_channel);
	}
	return data::Value();
}

////////////////////////////////////////////////////////////////////////////////

data::Value ChSettingsProtectionSetPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_CHANNEL_PROTECTION_OVP_LIMIT ||
		id == DATA_ID_CHANNEL_PROTECTION_OCP_LIMIT ||
		id == DATA_ID_CHANNEL_PROTECTION_OPP_LIMIT) {
		return limit;
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

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
	if (id == DATA_ID_CHANNEL_TEMP_STATUS) {
		return data::Value(2);
	}
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
	temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + g_channel->index - 1];
	if (tempSensor.isInstalled()) {
		if (tempSensor.isTestOK()) {
	        if (id == DATA_ID_CHANNEL_TEMP_STATUS) {
		        return data::Value(1);
	        }
	        if (id == DATA_ID_CHANNEL_TEMP) {
		        return data::Value(tempSensor.temperature, VALUE_TYPE_FLOAT_CELSIUS);
	        }
		} else {
	        if (id == DATA_ID_CHANNEL_TEMP_STATUS) {
		        return data::Value(0);
	        }
		}
	} else {
	    if (id == DATA_ID_CHANNEL_TEMP_STATUS) {
		    return data::Value(2);
	    }
	}
#endif

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
	NumericKeypadOptions options;

	options.editUnit = limit.getType();

	options.min = minLimit;
	options.max = maxLimit;
	options.def = defLimit;

	options.enableMaxButton();
	options.enableDefButton();
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, limit, options, onLimitSet);
}

void ChSettingsProtectionSetPage::onLevelSet(float value) {
	popPage();
	ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getActivePage();
	page->level = data::Value(value, page->level.getType());
}

void ChSettingsProtectionSetPage::editLevel() {
	NumericKeypadOptions options;

	options.editUnit = level.getType();

	options.min = minLevel;
	options.max = maxLevel;
	options.def = defLevel;

	options.enableMaxButton();
	options.enableDefButton();
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, level, options, onLevelSet);
}

void ChSettingsProtectionSetPage::onDelaySet(float value) {
	popPage();
	ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getActivePage();
	page->delay = data::Value(value, page->delay.getType());
}

void ChSettingsProtectionSetPage::editDelay() {
	NumericKeypadOptions options;

	options.editUnit = delay.getType();

	options.min = minDelay;
	options.max = maxDelay;
	options.def = defaultDelay;

	options.enableMaxButton();
	options.enableDefButton();
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, delay, options, onDelaySet);
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsOvpProtectionPage::ChSettingsOvpProtectionPage() {
	origState = state = g_channel->prot_conf.flags.u_state ? 1 : 0;

	origLimit = limit = data::Value(channel_dispatcher::getULimit(*g_channel), VALUE_TYPE_FLOAT_VOLT);
	minLimit = channel_dispatcher::getUMin(*g_channel);
	maxLimit = channel_dispatcher::getUMax(*g_channel);
	defLimit = channel_dispatcher::getUMax(*g_channel);

	origLevel = level = data::Value(channel_dispatcher::getUProtectionLevel(*g_channel), VALUE_TYPE_FLOAT_VOLT);
	minLevel = channel_dispatcher::getUSet(*g_channel);
	maxLevel = channel_dispatcher::getUMax(*g_channel);
	defLevel = channel_dispatcher::getUMax(*g_channel);

	origDelay = delay = data::Value(g_channel->prot_conf.u_delay, VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OVP_MIN_DELAY;
	maxDelay = g_channel->OVP_MAX_DELAY;
	defaultDelay = g_channel->OVP_DEFAULT_DELAY;
}

void ChSettingsOvpProtectionPage::onSetParamsOk() {
	((ChSettingsOvpProtectionPage *)getActivePage())->setParams(false);
}

void ChSettingsOvpProtectionPage::setParams(bool checkLoad) {
	if (checkLoad &&
        g_channel->isOutputEnabled() &&
        util::less(limit.getFloat(), channel_dispatcher::getUMon(*g_channel), getPrecision(VALUE_TYPE_FLOAT_VOLT)) &&
        util::greaterOrEqual(channel_dispatcher::getIMon(*g_channel), 0, getPrecision(VALUE_TYPE_FLOAT_AMPER))) 
    {
		areYouSureWithMessage(PSTR("This change will affect current load."), onSetParamsOk);
	} else {
		channel_dispatcher::setVoltageLimit(*g_channel, limit.getFloat());
        channel_dispatcher::setOvpParameters(*g_channel, state, level.getFloat(), delay.getFloat());
        onSetFinish(checkLoad);
	}
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsOcpProtectionPage::ChSettingsOcpProtectionPage() {
	origState = state = g_channel->prot_conf.flags.i_state ? 1 : 0;

	origLimit = limit = data::Value(channel_dispatcher::getILimit(*g_channel), VALUE_TYPE_FLOAT_AMPER);
	minLimit = channel_dispatcher::getIMin(*g_channel);
	maxLimit = channel_dispatcher::getIMaxLimit(*g_channel);
	defLimit = maxLimit;

	origLevel = level = 0;

	origDelay = delay = data::Value(g_channel->prot_conf.i_delay, VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OCP_MIN_DELAY;
	maxDelay = g_channel->OCP_MAX_DELAY;
	defaultDelay = g_channel->OCP_DEFAULT_DELAY;
}

void ChSettingsOcpProtectionPage::onSetParamsOk() {
	((ChSettingsOcpProtectionPage *)getActivePage())->setParams(false);
}

void ChSettingsOcpProtectionPage::setParams(bool checkLoad) {
	if (checkLoad && g_channel->isOutputEnabled() && limit.getFloat() < channel_dispatcher::getIMon(*g_channel)) {
		areYouSureWithMessage(PSTR("This change will affect current load."), onSetParamsOk);
	} else {
		channel_dispatcher::setCurrentLimit(*g_channel, limit.getFloat());
        channel_dispatcher::setOcpParameters(*g_channel, state, delay.getFloat());
		onSetFinish(checkLoad);
	}
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsOppProtectionPage::ChSettingsOppProtectionPage() {
	origState = state = g_channel->prot_conf.flags.p_state ? 1 : 0;

	origLimit = limit = data::Value(channel_dispatcher::getPowerLimit(*g_channel), VALUE_TYPE_FLOAT_WATT);
	minLimit = channel_dispatcher::getPowerMinLimit(*g_channel);
	maxLimit = channel_dispatcher::getPowerMaxLimit(*g_channel);
	defLimit = channel_dispatcher::getPowerDefaultLimit(*g_channel);

	origLevel = level = data::Value(channel_dispatcher::getPowerProtectionLevel(*g_channel), VALUE_TYPE_FLOAT_WATT);
	minLevel = channel_dispatcher::getOppMinLevel(*g_channel);
	maxLevel = channel_dispatcher::getOppMaxLevel(*g_channel);
	defLevel = channel_dispatcher::getOppDefaultLevel(*g_channel);

	origDelay = delay = data::Value(g_channel->prot_conf.p_delay, VALUE_TYPE_FLOAT_SECOND);
	minDelay = g_channel->OPP_MIN_DELAY;
	maxDelay = g_channel->OPP_MAX_DELAY;
	defaultDelay = g_channel->OPP_DEFAULT_DELAY;
}

void ChSettingsOppProtectionPage::onSetParamsOk() {
	((ChSettingsOppProtectionPage *)getActivePage())->setParams(false);
}

void ChSettingsOppProtectionPage::setParams(bool checkLoad) {
	if (checkLoad && g_channel->isOutputEnabled()) {
		float pMon = channel_dispatcher::getUMon(*g_channel) * channel_dispatcher::getIMon(*g_channel);
		if (util::less(limit.getFloat(), pMon, getPrecision(VALUE_TYPE_FLOAT_WATT)) &&
            util::greaterOrEqual(channel_dispatcher::getIMon(*g_channel), 0, getPrecision(VALUE_TYPE_FLOAT_AMPER))) 
        {
			areYouSureWithMessage(PSTR("This change will affect current load."), onSetParamsOk);
			return;
		}
	}

	channel_dispatcher::setPowerLimit(*g_channel, limit.getFloat());
    channel_dispatcher::setOppParameters(*g_channel, state, level.getFloat(), delay.getFloat());
	onSetFinish(checkLoad);
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsOtpProtectionPage::ChSettingsOtpProtectionPage() {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
	origState = state = temperature::getChannelSensorState(g_channel) ? 1 : 0;

	origLimit = limit = 0;
	minLimit = 0;
	maxLimit = 0;
	defLimit = 0;

	origLevel = level = data::Value(temperature::getChannelSensorLevel(g_channel), VALUE_TYPE_FLOAT_CELSIUS);
	minLevel = OTP_AUX_MIN_LEVEL;
	maxLevel = OTP_AUX_MAX_LEVEL;
	defLevel = OTP_AUX_DEFAULT_LEVEL;

	origDelay = delay = data::Value(temperature::getChannelSensorDelay(g_channel), VALUE_TYPE_FLOAT_SECOND);
	minDelay = OTP_AUX_MIN_DELAY;
	maxDelay = OTP_AUX_MAX_DELAY;
	defaultDelay = OTP_CH_DEFAULT_DELAY;
#endif
}

void ChSettingsOtpProtectionPage::setParams(bool checkLoad) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
	channel_dispatcher::setOtpParameters(*g_channel, state, level.getFloat(), delay.getFloat());
	onSetFinish(checkLoad);
#endif
}

}
}
} // namespace eez::psu::gui

#endif