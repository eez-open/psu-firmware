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
#include "channel_dispatcher.h"
#include "trigger.h"

#include "gui_page_ch_settings_adv.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

data::Value ChSettingsAdvPage::getData(const data::Cursor &cursor, uint8_t id) {
	if (id == DATA_ID_CHANNEL_RANGES_SUPPORTED) {
		return g_channel->hasSupportForCurrentDualRange() ? 1 : 0;
	}
	return data::Value();
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsAdvLRipplePage::ChSettingsAdvLRipplePage() {
	origAutoMode = autoMode = g_channel->isLowRippleAutoEnabled();
	origStatus = status = g_channel->isLowRippleEnabled();
}

data::Value ChSettingsAdvLRipplePage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_MAX_DISSIPATION) {
		return data::Value(g_channel->SOA_POSTREG_PTOT, VALUE_TYPE_FLOAT_WATT, g_channel->index-1);
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_CALCULATED_DISSIPATION) {
        Channel &channel = Channel::get(g_channel->index - 1);
		return data::Value(channel_dispatcher::getIMon(channel) * (g_channel->SOA_VIN - channel_dispatcher::getUMon(channel)), VALUE_TYPE_FLOAT_WATT, g_channel->index-1);
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_AUTO_MODE) {
		return autoMode;
	}

	if (id == DATA_ID_CHANNEL_LRIPPLE_IS_ALLOWED) {
		return g_channel->isLowRippleAllowed(micros());
	}

    if (id == DATA_ID_CHANNEL_LRIPPLE_STATUS) {
		return g_channel->isLowRippleAllowed(micros()) ? status : false;
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
	return (origAutoMode != autoMode || g_channel->isLowRippleAllowed(micros()) && origStatus != status) ? 1 : 0;
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

data::Value ChSettingsAdvRemotePage::getData(const data::Cursor &cursor, uint8_t id) {
	if (id == DATA_ID_CHANNEL_RSENSE_STATUS) {
		return g_channel->isRemoteSensingEnabled() ? 1 : 0;
	}
	if (id == DATA_ID_CHANNEL_RPROG_INSTALLED) {
		return data::Value(g_channel->getFeatures() & CH_FEATURE_RPROG ? 1 : 0);
	}
	return data::Value();
}

void ChSettingsAdvRemotePage::toggleSense() {
	channel_dispatcher::remoteSensingEnable(*g_channel, !g_channel->isRemoteSensingEnabled());
}

void ChSettingsAdvRemotePage::toggleProgramming() {
	g_channel->remoteProgrammingEnable(!g_channel->isRemoteProgrammingEnabled());
}

////////////////////////////////////////////////////////////////////////////////

data::Value ChSettingsAdvRangesPage::getData(const data::Cursor &cursor, uint8_t id) {
	if (id == DATA_ID_CHANNEL_RANGES_MODE) {
		return data::Value(g_channel->getCurrentRangeSelectionMode(),
            data::ENUM_DEFINITION_CHANNEL_CURRENT_RANGE_SELECTION_MODE);
	}
	
    if (id == DATA_ID_CHANNEL_RANGES_AUTO_RANGING) {
		return data::Value(g_channel->isAutoSelectCurrentRangeEnabled() ? 1 : 0);
	}

    if (id == DATA_ID_CHANNEL_RANGES_CURRENTLY_SELECTED) {
		return data::Value(g_channel->flags.currentCurrentRange,
            data::ENUM_DEFINITION_CHANNEL_CURRENT_RANGE);
	}
	
    return data::Value();
}

void ChSettingsAdvRangesPage::onModeSet(uint8_t value) {
	popPage();
    g_channel->setCurrentRangeSelectionMode((CurrentRangeSelectionMode)value);
}

void ChSettingsAdvRangesPage::selectMode() {
    pushSelectFromEnumPage(data::g_channelCurrentRangeSelectionModeEnumDefinition, g_channel->getCurrentRangeSelectionMode(), 0, onModeSet);
}

void ChSettingsAdvRangesPage::toggleAutoRanging() {
    g_channel->enableAutoSelectCurrentRange(!g_channel->isAutoSelectCurrentRangeEnabled());
}

////////////////////////////////////////////////////////////////////////////////

int ChSettingsAdvCouplingPage::selectedMode = 0;

data::Value ChSettingsAdvCouplingPage::getData(const data::Cursor &cursor, uint8_t id) {
	if (id == DATA_ID_CHANNEL_COUPLING_SELECTED_MODE) {
		return data::Value(selectedMode);
	}

	return data::Value();
}

void ChSettingsAdvCouplingPage::uncouple() {
    channel_dispatcher::setType(channel_dispatcher::TYPE_NONE);
    profile::save();
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
            profile::save();
	        infoMessageP(PSTR("Channels coupled in parallel!"), popPage);
        }
    }
}

void ChSettingsAdvCouplingPage::setSeries() {
    if (selectedMode == 1) {
        if (channel_dispatcher::setType(channel_dispatcher::TYPE_SERIES)) {
            profile::save();
    	    infoMessageP(PSTR("Channels coupled in series!"), popPage);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ChSettingsAdvTrackingPage::toggleTrackingMode() {
    if (channel_dispatcher::isTracked()) {
        channel_dispatcher::setType(channel_dispatcher::TYPE_NONE);
        profile::save();
        infoMessageP(PSTR("Tracking disabled!"));
    } else {
        channel_dispatcher::setType(channel_dispatcher::TYPE_TRACKED);
        profile::save();
	    infoMessageP(PSTR("Tracking enabled!"));
    }
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsAdvViewPage::ChSettingsAdvViewPage() {
	origDisplayValue1 = displayValue1 = g_channel->flags.displayValue1;
	origDisplayValue2 = displayValue2 = g_channel->flags.displayValue2;
    origYTViewRate = ytViewRate = g_channel->ytViewRate;
}

data::Value ChSettingsAdvViewPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_CHANNEL_DISPLAY_VIEW_SETTINGS_DISPLAY_VALUE1) {
		return data::Value(displayValue1, data::ENUM_DEFINITION_CHANNEL_DISPLAY_VALUE);
	}

	if (id == DATA_ID_CHANNEL_DISPLAY_VIEW_SETTINGS_DISPLAY_VALUE2) {
		return data::Value(displayValue2, data::ENUM_DEFINITION_CHANNEL_DISPLAY_VALUE);
	}

    if (id == DATA_ID_CHANNEL_DISPLAY_VIEW_SETTINGS_YT_VIEW_RATE) {
		return data::Value(ytViewRate, VALUE_TYPE_FLOAT_SECOND);
	}

	return data::Value();
}

bool ChSettingsAdvViewPage::isDisabledDisplayValue1(uint8_t value) {
	ChSettingsAdvViewPage *page = (ChSettingsAdvViewPage *)getPreviousPage();
    return value == page->displayValue2;
}

void ChSettingsAdvViewPage::onDisplayValue1Set(uint8_t value) {
	popPage();
	ChSettingsAdvViewPage *page = (ChSettingsAdvViewPage *)getActivePage();
	page->displayValue1 = value;
}

void ChSettingsAdvViewPage::editDisplayValue1() {
    pushSelectFromEnumPage(data::g_channelDisplayValueEnumDefinition, displayValue1, isDisabledDisplayValue1, onDisplayValue1Set);
}

bool ChSettingsAdvViewPage::isDisabledDisplayValue2(uint8_t value) {
	ChSettingsAdvViewPage *page = (ChSettingsAdvViewPage *)getPreviousPage();
    return value == page->displayValue1;
}

void ChSettingsAdvViewPage::onDisplayValue2Set(uint8_t value) {
	popPage();
	ChSettingsAdvViewPage *page = (ChSettingsAdvViewPage *)getActivePage();
	page->displayValue2 = value;
}

void ChSettingsAdvViewPage::editDisplayValue2() {
    pushSelectFromEnumPage(data::g_channelDisplayValueEnumDefinition, displayValue2, isDisabledDisplayValue2, onDisplayValue2Set);
}

void ChSettingsAdvViewPage::onYTViewRateSet(float value) {
	popPage();
	ChSettingsAdvViewPage *page = (ChSettingsAdvViewPage *)getActivePage();
	page->ytViewRate = value;
}

void ChSettingsAdvViewPage::swapDisplayValues() {
    uint8_t temp = displayValue1;
    displayValue1 = displayValue2;
    displayValue2 = temp;
}

void ChSettingsAdvViewPage::editYTViewRate() {
	NumericKeypadOptions options;

	options.editUnit = VALUE_TYPE_FLOAT_SECOND;

	options.min = GUI_YT_VIEW_RATE_MIN;
    options.max = GUI_YT_VIEW_RATE_MAX;
	options.def = GUI_YT_VIEW_RATE_DEFAULT;

	options.enableDefButton();
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, data::Value(ytViewRate, VALUE_TYPE_FLOAT_SECOND), options, onYTViewRateSet);
}

int ChSettingsAdvViewPage::getDirty() {
	return (origDisplayValue1 != displayValue1 || origDisplayValue2 != displayValue2 || origYTViewRate != ytViewRate) ? 1 : 0;
}

void ChSettingsAdvViewPage::set() {
	if (getDirty()) {
        channel_dispatcher::setDisplayViewSettings(*g_channel, displayValue1, displayValue2, ytViewRate);
		profile::save();
		infoMessageP(PSTR("View settings changed!"), actions[ACTION_ID_SHOW_CH_SETTINGS_ADV]);
	}
}

}
}
} // namespace eez::psu::gui

#endif