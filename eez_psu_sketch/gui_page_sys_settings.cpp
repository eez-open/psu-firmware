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

#include "persist_conf.h"
#include "ethernet.h"
#include "temperature.h"
#include "profile.h"

#include "gui_data_snapshot.h"
#include "gui_page_sys_settings.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

SysSettingsDateTimePage::SysSettingsDateTimePage() {
	dateTime = origDateTime = datetime::DateTime::now();
	timeZone = origTimeZone = persist_conf::devConf.time_zone;
	dst = origDst = persist_conf::devConf.flags.dst;
}

void SysSettingsDateTimePage::takeSnapshot(data::Snapshot *snapshot) {
	SetPage::takeSnapshot(snapshot);

	snapshot->flags.switch1 = dst;
}

data::Value SysSettingsDateTimePage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	data::Value value = SetPage::getData(cursor, id, snapshot);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_SYS_INFO_DATE_TIME_YEAR) {
		return data::Value(dateTime.year, data::VALUE_TYPE_YEAR);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MONTH) {
		return data::Value(dateTime.month, data::VALUE_TYPE_MONTH);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_DAY) {
		return data::Value(dateTime.day, data::VALUE_TYPE_DAY);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_HOUR) {
		return data::Value(dateTime.hour, data::VALUE_TYPE_HOUR);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MINUTE) {
		return data::Value(dateTime.minute, data::VALUE_TYPE_MINUTE);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		return data::Value(dateTime.second, data::VALUE_TYPE_SECOND);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_TIME_ZONE) {
		return data::Value(timeZone, data::VALUE_TYPE_TIME_ZONE);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_DST) {
		return data::Value(snapshot->flags.switch1 ? 1 : 0);
	}

	return data::Value();
}

void SysSettingsDateTimePage::edit() {
	DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
	int id = widget->data;

	numeric_keypad::Options options;

	options.editUnit = data::VALUE_TYPE_INT;

	options.flags.genericNumberKeypad = true;
	options.flags.maxButtonEnabled = false;
	options.flags.defButtonEnabled = false;
	options.flags.signButtonEnabled = false;
	options.flags.dotButtonEnabled = false;

	const char *label = 0;

	if (id == DATA_ID_SYS_INFO_DATE_TIME_YEAR) {
		label = "Year (2016-2036): ";
		options.min = 2016;
		options.max = 2036;
		options.def = 2016;
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MONTH) {
		label = "Month (1-12): ";
		options.min = 1;
		options.max = 12;
		options.def = 1;
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_DAY) {
		label = "Day (1-31): ";
		options.min = 1;
		options.max = 31;
		options.def = 1;
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_HOUR) {
		label = "Hour (0-23): ";
		options.min = 0;
		options.max = 23;
		options.def = 12;
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MINUTE) {
		label = "Minute (0-59): ";
		options.min = 0;
		options.max = 59;
		options.def = 0;
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		label = "Second (0-59): ";
		options.min = 0;
		options.max = 59;
		options.def = 0;
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_TIME_ZONE) {
		label = "Time zone: ";
		options.min = -12.00;
		options.max = 14.00;
		options.def = 0;
		options.editUnit = data::VALUE_TYPE_TIME_ZONE;
		options.flags.dotButtonEnabled = true;
		options.flags.signButtonEnabled = true;
	}

	if (label) {
		editDataId = id;
		numeric_keypad::start(label, options, onSetValue);
	}
}

void SysSettingsDateTimePage::toggleDst() {
	dst = dst ? 0 : 1;
}

void SysSettingsDateTimePage::setValue(float value) {
	if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_YEAR) {
		dateTime.year = uint16_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_MONTH) {
		dateTime.month = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_DAY) {
		dateTime.day = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_HOUR) {
		dateTime.hour = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_MINUTE) {
		dateTime.minute = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		dateTime.second = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		dateTime.second = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_TIME_ZONE) {
		timeZone = int16_t(roundf(value * 100));
	}
}

int SysSettingsDateTimePage::getDirty() {
	return (dateTime != origDateTime || timeZone != origTimeZone || dst != origDst) ? 1 : 0;
}

void SysSettingsDateTimePage::set() {
    if (!datetime::isValidDate(uint8_t(dateTime.year - 2000), dateTime.month, dateTime.day)) {
        errorMessageP(PSTR("Invalid date!"));
        return;
    }

    if (!datetime::isValidTime(dateTime.hour, dateTime.minute, dateTime.second)) {
        errorMessageP(PSTR("Invalid time!"));
        popPage();
		return;
    }
	
	if (dateTime != origDateTime) {
		if (!datetime::setDateTime(uint8_t(dateTime.year - 2000), dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second)) {
			errorMessageP(PSTR("Failed to set system date and time!"));
			return;
		}
	}

	if (timeZone != origTimeZone || dst != origDst) {
		persist_conf::devConf.time_zone = timeZone;
		persist_conf::devConf.flags.dst = dst;
		if (!persist_conf::saveDevice()) {
	        errorMessageP(PSTR("Failed to set time zone and DST!"));
			return;
		}
		
		if (dateTime == origDateTime) {
			event_queue::pushEvent(event_queue::EVENT_INFO_SYSTEM_DATE_TIME_CHANGED);
		}
	}

	infoMessageP(PSTR("Date and time settings saved!"));
	return;
}

////////////////////////////////////////////////////////////////////////////////

void SysSettingsEthernetPage::takeSnapshot(data::Snapshot *snapshot) {
    snapshot->flags.switch1 = persist_conf::isEthernetEnabled() ? 1 : 0;
}

data::Value SysSettingsEthernetPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
    if (id == DATA_ID_SYS_ETHERNET_ENABLED) {
        return data::Value(snapshot->flags.switch1);
    }

    if (id == DATA_ID_SYS_ETHERNET_STATUS) {
        return data::Value(ethernet::test_result);
    }

    if (id == DATA_ID_SYS_ETHERNET_IP_ADDRESS) {
        return data::Value(ethernet::getIpAddress(), data::VALUE_TYPE_IP_ADDRESS);
    }

    if (id == DATA_ID_SYS_ETHERNET_SCPI_PORT) {
        return TCP_PORT;
    }

    return data::Value();
}

static void enableEthernet(bool enable) {
    persist_conf::enableEthernet(enable);
    longInfoMessageP(
        PSTR("Turn off and on power or"),
        PSTR("press reset to apply changes!"));
}

void SysSettingsEthernetPage::enable() {
    enableEthernet(true);
}

void SysSettingsEthernetPage::disable() {
    enableEthernet(false);
}

////////////////////////////////////////////////////////////////////////////////

void SysSettingsProtectionsPage::takeSnapshot(data::Snapshot *snapshot) {
    snapshot->flags.switch1 = persist_conf::isOutputProtectionCoupleEnabled() ? 1 : 0;
    snapshot->flags.switch2 = persist_conf::isShutdownWhenProtectionTrippedEnabled() ? 1 : 0;
    snapshot->flags.switch3 = persist_conf::isForceDisablingAllOutputsOnPowerUpEnabled() ? 1 : 0;
}

data::Value SysSettingsProtectionsPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
    if (id == DATA_ID_SYS_OUTPUT_PROTECTION_COUPLED) {
        return data::Value(snapshot->flags.switch1);
    }

    if (id == DATA_ID_SYS_SHUTDOWN_WHEN_PROTECTION_TRIPPED) {
        return data::Value(snapshot->flags.switch2);
    }

    if (id == DATA_ID_SYS_FORCE_DISABLING_ALL_OUTPUTS_ON_POWER_UP) {
        return data::Value(snapshot->flags.switch3);
    }

    return data::Value();
}

void SysSettingsProtectionsPage::toggleOutputProtectionCouple() {
    if (persist_conf::isOutputProtectionCoupleEnabled()) {
        if (persist_conf::enableOutputProtectionCouple(false)) {
            infoMessageP(PSTR("Output protection decoupled!"));
        }
    } else {
        if (persist_conf::enableOutputProtectionCouple(true)) {
            infoMessageP(PSTR("Output protection coupled!"));
        }
    }
}

void SysSettingsProtectionsPage::toggleShutdownWhenProtectionTripped() {
    if (persist_conf::isShutdownWhenProtectionTrippedEnabled()) {
        if (persist_conf::enableShutdownWhenProtectionTripped(false)) {
            infoMessageP(PSTR("Shutdown when tripped disabled!"));
        }
    } else {
        if (persist_conf::enableShutdownWhenProtectionTripped(true)) {
            infoMessageP(PSTR("Shutdown when tripped enabled!"));
        }
    }
}

void SysSettingsProtectionsPage::toggleForceDisablingAllOutputsOnPowerUp() {
    if (persist_conf::isForceDisablingAllOutputsOnPowerUpEnabled()) {
        if (persist_conf::enableForceDisablingAllOutputsOnPowerUp(false)) {
            infoMessageP(PSTR("Force disabling outputs disabled!"));
        }
    } else {
        if (persist_conf::enableForceDisablingAllOutputsOnPowerUp(true)) {
            infoMessageP(PSTR("Force disabling outputs enabled!"));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

SysSettingsAuxOtpPage::SysSettingsAuxOtpPage() {
	origState = state = temperature::sensors[temp_sensor::AUX].prot_conf.state ? 1 : 0;

	origLevel = level = data::Value(temperature::sensors[temp_sensor::AUX].prot_conf.level, data::VALUE_TYPE_FLOAT_CELSIUS);
	minLevel = OTP_AUX_MIN_LEVEL;
	maxLevel = OTP_AUX_MAX_LEVEL;
	defLevel = OTP_AUX_DEFAULT_LEVEL;

	origDelay = delay = data::Value(temperature::sensors[temp_sensor::AUX].prot_conf.delay, data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = OTP_AUX_MIN_DELAY;
	maxDelay = OTP_AUX_MAX_DELAY;
	defaultDelay = OTP_CH_DEFAULT_DELAY;
}

void SysSettingsAuxOtpPage::takeSnapshot(data::Snapshot *snapshot) {
	SetPage::takeSnapshot(snapshot);

	snapshot->flags.switch1 = state;
    snapshot->flags.switch2 = temperature::sensors[temp_sensor::AUX].isTripped() ? 1 : 0;
}

data::Value SysSettingsAuxOtpPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	data::Value value = SetPage::getData(cursor, id, snapshot);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_SYS_TEMP_AUX_OTP_STATE) {
		return snapshot->flags.switch1;
	}

    if (id == DATA_ID_SYS_TEMP_AUX_OTP_LEVEL) {
		return level;
	}

	if (id == DATA_ID_SYS_TEMP_AUX_OTP_DELAY) {
		return delay;
	}

	if (id == DATA_ID_SYS_TEMP_AUX_OTP_IS_TRIPPED) {
		return snapshot->flags.switch2;
	}

	return data::Value();
}

int SysSettingsAuxOtpPage::getDirty() {
	return (origState != state || origLevel != level || origDelay != delay) ? 1 : 0;
}

void SysSettingsAuxOtpPage::set() {
	if (getDirty()) {
		setParams();
	}
}

void SysSettingsAuxOtpPage::toggleState() {
	state = state ? 0 : 1;
}


void SysSettingsAuxOtpPage::onLevelSet(float value) {
	popPage();
	SysSettingsAuxOtpPage *page = (SysSettingsAuxOtpPage *)getActivePage();
	page->level = data::Value(value, page->level.getType());
}

void SysSettingsAuxOtpPage::editLevel() {
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

void SysSettingsAuxOtpPage::onDelaySet(float value) {
	popPage();
	SysSettingsAuxOtpPage *page = (SysSettingsAuxOtpPage *)getActivePage();
	page->delay = data::Value(value, page->delay.getType());
}

void SysSettingsAuxOtpPage::editDelay() {
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

void SysSettingsAuxOtpPage::setParams() {
    temperature::sensors[temp_sensor::AUX].prot_conf.state = state ? true : false;
    temperature::sensors[temp_sensor::AUX].prot_conf.level = level.getFloat();
    temperature::sensors[temp_sensor::AUX].prot_conf.delay = delay.getFloat();

    profile::save();
    infoMessageP(PSTR("Aux temp. protection changed!"), popPage);
}

void SysSettingsAuxOtpPage::clear() {
    temperature::sensors[temp_sensor::AUX].clearProtection();
    infoMessageP(PSTR("Cleared!"), popPage);
}

}
}
} // namespace eez::psu::gui
