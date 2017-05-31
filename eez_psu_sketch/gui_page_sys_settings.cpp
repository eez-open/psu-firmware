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

#include "persist_conf.h"
#include "ethernet.h"
#include "temperature.h"
#include "profile.h"
#if OPTION_ENCODER
#include "encoder.h"
#endif
#include "util.h"
#if OPTION_ETHERNET
#include "ntp.h"
#endif

#include "gui_page_sys_settings.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

SysSettingsDateTimePage::SysSettingsDateTimePage() {
#if OPTION_ETHERNET
    ntpEnabled = origNtpEnabled = persist_conf::isNtpEnabled();
    strcpy(ntpServer, persist_conf::devConf2.ntpServer);
    strcpy(origNtpServer, persist_conf::devConf2.ntpServer);
#else
    ntpEnabled = origNtpEnabled = false;
    strcpy(ntpServer, '');
    strcpy(origNtpServer, '');
#endif

    dateTime = origDateTime = datetime::DateTime::now();
	timeZone = origTimeZone = persist_conf::devConf.time_zone;
	dstRule = origDstRule = (datetime::DstRule)persist_conf::devConf2.dstRule;
}

data::Value SysSettingsDateTimePage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

    if (id == DATA_ID_NTP_ENABLED) {
        return data::Value(ntpEnabled ? 1 : 0);
    }

    if (ntpEnabled) {
        uint32_t nowUtc = datetime::nowUtc();
        uint32_t nowLocal = datetime::utcToLocal(nowUtc, timeZone, dstRule);

        if (id == DATA_ID_NTP_SERVER) {
            return ntpServer[0] ? ntpServer : "<not specified>";
        }

        if (id == DATA_ID_DATE_TIME_DATE) {
            return data::Value(nowLocal, VALUE_TYPE_DATE);
        }

        if (id == DATA_ID_DATE_TIME_TIME) {
            return data::Value(nowLocal, VALUE_TYPE_TIME);
        }
    } else {
	    if (id == DATA_ID_DATE_TIME_YEAR) {
		    return data::Value(dateTime.year, VALUE_TYPE_YEAR);
	    }
    
        if (id == DATA_ID_DATE_TIME_MONTH) {
		    return data::Value(dateTime.month, VALUE_TYPE_MONTH);
	    }
    
        if (id == DATA_ID_DATE_TIME_DAY) {
		    return data::Value(dateTime.day, VALUE_TYPE_DAY);
	    }
    
        if (id == DATA_ID_DATE_TIME_HOUR) {
		    return data::Value(dateTime.hour, VALUE_TYPE_HOUR);
	    }
    
        if (id == DATA_ID_DATE_TIME_MINUTE) {
		    return data::Value(dateTime.minute, VALUE_TYPE_MINUTE);
	    }
    
        if (id == DATA_ID_DATE_TIME_SECOND) {
		    return data::Value(dateTime.second, VALUE_TYPE_SECOND);
	    }
    }
    
    if (id == DATA_ID_DATE_TIME_TIME_ZONE) {
		return data::Value(timeZone, VALUE_TYPE_TIME_ZONE);
	}
    
    if (id == DATA_ID_DATE_TIME_DST) {
		return data::Value(dstRule, data::ENUM_DEFINITION_DST_RULE);
	}

	return data::Value();
}

void SysSettingsDateTimePage::toggleNtp() {
    ntpEnabled = !ntpEnabled;
}

void SysSettingsDateTimePage::onSetNtpServer(char *value) {
	SysSettingsDateTimePage *page = (SysSettingsDateTimePage*)getPreviousPage();
    strcpy(page->ntpServer, value);

    popPage();
}

void SysSettingsDateTimePage::editNtpServer() {
    Keypad::startPush(0, ntpServer, 32, false, onSetNtpServer, popPage);
}

void SysSettingsDateTimePage::edit() {
	DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
	int id = widget->data;

	NumericKeypadOptions options;

	options.editUnit = VALUE_TYPE_INT;

	const char *label = 0;

    data::Value value;

    if (id == DATA_ID_DATE_TIME_YEAR) {
		label = "Year (2016-2036): ";
		options.min = 2017;
		options.max = 2036;
		options.def = 2017;
        value = data::Value((int)dateTime.year);
	} else if (id == DATA_ID_DATE_TIME_MONTH) {
		label = "Month (1-12): ";
		options.min = 1;
		options.max = 12;
		options.def = 1;
        value = data::Value((int)dateTime.month);
	} else if (id == DATA_ID_DATE_TIME_DAY) {
		label = "Day (1-31): ";
		options.min = 1;
		options.max = 31;
		options.def = 1;
        value = data::Value((int)dateTime.day);
	} else if (id == DATA_ID_DATE_TIME_HOUR) {
		label = "Hour (0-23): ";
		options.min = 0;
		options.max = 23;
		options.def = 12;
        value = data::Value((int)dateTime.hour);
	} else if (id == DATA_ID_DATE_TIME_MINUTE) {
		label = "Minute (0-59): ";
		options.min = 0;
		options.max = 59;
		options.def = 0;
        value = data::Value((int)dateTime.minute);
	} else if (id == DATA_ID_DATE_TIME_SECOND) {
		label = "Second (0-59): ";
		options.min = 0;
		options.max = 59;
		options.def = 0;
        value = data::Value((int)dateTime.second);
	} else if (id == DATA_ID_DATE_TIME_TIME_ZONE) {
		label = "Time zone: ";
		options.min = -12.00;
		options.max = 14.00;
		options.def = 0;
		options.editUnit = VALUE_TYPE_TIME_ZONE;
		options.flags.dotButtonEnabled = true;
		options.flags.signButtonEnabled = true;
	}

	if (label) {
		editDataId = id;
		NumericKeypad::start(label, value, options, onSetValue);
	}
}

void SysSettingsDateTimePage::onDstRuleSet(uint8_t value) {
    popPage();
	SysSettingsDateTimePage *page = (SysSettingsDateTimePage*)getActivePage();
    page->dstRule = (datetime::DstRule)value;
}

void SysSettingsDateTimePage::selectDstRule() {
    pushSelectFromEnumPage(data::g_dstRuleEnumDefinition, dstRule, 0, onDstRuleSet);
}

void SysSettingsDateTimePage::setValue(float value) {
	if (editDataId == DATA_ID_DATE_TIME_YEAR) {
		dateTime.year = uint16_t(value);
	} else if (editDataId == DATA_ID_DATE_TIME_MONTH) {
		dateTime.month = uint8_t(value);
	} else if (editDataId == DATA_ID_DATE_TIME_DAY) {
		dateTime.day = uint8_t(value);
	} else if (editDataId == DATA_ID_DATE_TIME_HOUR) {
		dateTime.hour = uint8_t(value);
	} else if (editDataId == DATA_ID_DATE_TIME_MINUTE) {
		dateTime.minute = uint8_t(value);
	} else if (editDataId == DATA_ID_DATE_TIME_SECOND) {
		dateTime.second = uint8_t(value);
	} else if (editDataId == DATA_ID_DATE_TIME_SECOND) {
		dateTime.second = uint8_t(value);
	} else if (editDataId == DATA_ID_DATE_TIME_TIME_ZONE) {
		timeZone = int16_t(roundf(value * 100));
	}
}

int SysSettingsDateTimePage::getDirty() {
	if (ntpEnabled != origNtpEnabled) {
        return 1;
    }
    
    if (ntpEnabled) {
        if (ntpServer[0] && strcmp(ntpServer, origNtpServer)) {
            return 1;
        }
    } else {
        if (dateTime != origDateTime) {
            return 1;
        }
    }

    return (timeZone != origTimeZone || dstRule != origDstRule) ? 1 : 0;
}

#if OPTION_ETHERNET

void SysSettingsDateTimePage::checkTestNtpServerStatus() {
    bool testResult;
    if (ntp::isTestNtpServerDone(testResult)) {
        popPage();

        if (testResult) {
            SysSettingsDateTimePage *page = (SysSettingsDateTimePage*)getActivePage();
            page->doSet();
        } else {
            errorMessageP(PSTR("Unable to connect to NTP server!"));
        }
    }
}

void SysSettingsDateTimePage::testNtpServer() {
    ntp::testNtpServer(ntpServer);
    showAsyncOperationInProgress("Testing NTP server...", checkTestNtpServerStatus);
}
#endif


void SysSettingsDateTimePage::set() {
    if (getDirty()) {
#if OPTION_ETHERNET
        if (ntpEnabled && strcmp(ntpServer, origNtpServer)) {
            testNtpServer();
            return;
        }
#endif
        doSet();
    }
}

void SysSettingsDateTimePage::doSet() {
    if (!ntpEnabled) {
        if (!datetime::isValidDate(uint8_t(dateTime.year - 2000), dateTime.month, dateTime.day)) {
            errorMessageP(PSTR("Invalid date!"));
            return;
        }

        if (!datetime::isValidTime(dateTime.hour, dateTime.minute, dateTime.second)) {
            errorMessageP(PSTR("Invalid time!"));
            popPage();
		    return;
        }
    }

#if OPTION_ETHERNET
    bool callNtpReset;
    if (ntpEnabled != origNtpEnabled || strcmp(ntpServer, origNtpServer)) {
        persist_conf::setNtpSettings(ntpEnabled, ntpServer);
        callNtpReset = true;
    }
#endif

    if (dstRule != origDstRule) {
		persist_conf::devConf2.dstRule = dstRule;
		persist_conf::saveDevice2();
    }

    if (!ntpEnabled && dateTime != origDateTime) {
		datetime::setDateTime(uint8_t(dateTime.year - 2000), dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second);
    }

	if (timeZone != origTimeZone) {
		persist_conf::devConf.time_zone = timeZone;
		persist_conf::saveDevice();
	}

#if OPTION_ETHERNET
    ntp::reset();
#endif

    if (ntpEnabled || dateTime == origDateTime) {
        event_queue::pushEvent(event_queue::EVENT_INFO_SYSTEM_DATE_TIME_CHANGED);
    }

    infoMessageP(PSTR("Date and time settings saved!"), popPage);
	return;
}

////////////////////////////////////////////////////////////////////////////////

#if OPTION_ETHERNET

SysSettingsEthernetPage::SysSettingsEthernetPage() {
    m_enabledOrig = m_enabled = persist_conf::isEthernetEnabled();
    m_dhcpEnabledOrig = m_dhcpEnabled = persist_conf::isEthernetDhcpEnabled();
    m_ipAddressOrig = m_ipAddress = persist_conf::devConf2.ethernetIpAddress;
    m_dnsOrig = m_dns = persist_conf::devConf2.ethernetDns;
    m_gatewayOrig = m_gateway = persist_conf::devConf2.ethernetGateway;
    m_subnetMaskOrig = m_subnetMask = persist_conf::devConf2.ethernetSubnetMask;
    m_scpiPortOrig = m_scpiPort = persist_conf::devConf2.ethernetScpiPort;
}

data::Value SysSettingsEthernetPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

    if (id == DATA_ID_ETHERNET_ENABLED) {
        return data::Value(m_enabled ? 1 : 0);
    }

    if (id == DATA_ID_ETHERNET_DHCP) {
        return data::Value(m_dhcpEnabled ? 1 : 0);
    }

    if (m_dhcpEnabled) {
        if (id == DATA_ID_ETHERNET_IP_ADDRESS) {
            return data::Value(ethernet::getIpAddress(), VALUE_TYPE_IP_ADDRESS);
        }
    }

    if (id == DATA_ID_ETHERNET_SCPI_PORT) {
        return data::Value((uint16_t)m_scpiPort, VALUE_TYPE_PORT);
    }

    if (id == DATA_ID_ETHERNET_MAC) {
        return data::Value(0, VALUE_TYPE_MAC_ADDRESS);
    }

    return data::Value();
}

void SysSettingsEthernetPage::toggle() {
    m_enabled = !m_enabled;
}

void SysSettingsEthernetPage::toggleDhcp() {
    m_dhcpEnabled = !m_dhcpEnabled;
}

void SysSettingsEthernetPage::editStaticAddress() {
    pushPage(PAGE_ID_SYS_SETTINGS_ETHERNET_STATIC);
}

void SysSettingsEthernetPage::onScpiPortSet(float value) {
    popPage();
    SysSettingsEthernetPage *page = (SysSettingsEthernetPage *)getActivePage();
    page->m_scpiPort = (uint16_t)value;
}

void SysSettingsEthernetPage::editScpiPort() {
	NumericKeypadOptions options;

	options.editUnit = VALUE_TYPE_PORT;

	options.min = 0;
	options.max = 65535;
	options.def = TCP_PORT;

    options.enableDefButton();

	NumericKeypad::start(0, data::Value((int)m_scpiPort, VALUE_TYPE_PORT), options, (void (*)(float))onScpiPortSet);
}

int SysSettingsEthernetPage::getDirty() {
    return
        m_enabledOrig != m_enabled ||
        m_dhcpEnabledOrig != m_dhcpEnabled ||
        m_ipAddressOrig != m_ipAddress ||
        m_dnsOrig != m_dns ||
        m_gatewayOrig != m_gateway ||
        m_subnetMaskOrig != m_subnetMask ||
        m_scpiPortOrig != m_scpiPort;
}

void SysSettingsEthernetPage::set() {
    if (getDirty()) {
        if (persist_conf::setEthernetSettings(m_enabled, m_dhcpEnabled, m_ipAddress, m_dns, m_gateway, m_subnetMask, m_scpiPort)) {
            longInfoMessageP(PSTR("Turn off and on power or"), PSTR("press reset to apply changes!"), popPage);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

SysSettingsEthernetStaticPage::SysSettingsEthernetStaticPage() {
    SysSettingsEthernetPage *page = (SysSettingsEthernetPage *)getPreviousPage();

    m_ipAddressOrig = m_ipAddress = page->m_ipAddress;
    m_dnsOrig = m_dns = page->m_dns;
    m_gatewayOrig = m_gateway = page->m_gateway;
    m_subnetMaskOrig = m_subnetMask = page->m_subnetMask;
}

data::Value SysSettingsEthernetStaticPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

    if (id == DATA_ID_ETHERNET_IP_ADDRESS) {
        return data::Value(m_ipAddress, VALUE_TYPE_IP_ADDRESS);
    }

    if (id == DATA_ID_ETHERNET_DNS) {
        return data::Value(m_dns, VALUE_TYPE_IP_ADDRESS);
    }

    if (id == DATA_ID_ETHERNET_GATEWAY) {
        return data::Value(m_gateway, VALUE_TYPE_IP_ADDRESS);
    }

    if (id == DATA_ID_ETHERNET_SUBNET_MASK) {
        return data::Value(m_subnetMask, VALUE_TYPE_IP_ADDRESS);
    }

    return data::Value();
}

void SysSettingsEthernetStaticPage::onAddressSet(uint32_t address) {
    popPage();
    SysSettingsEthernetStaticPage *page = (SysSettingsEthernetStaticPage *)getActivePage();
    *page->m_editAddress = address;
}

void SysSettingsEthernetStaticPage::edit(uint32_t &address) {
    m_editAddress = &address;

	NumericKeypadOptions options;

	options.editUnit = VALUE_TYPE_IP_ADDRESS;

	NumericKeypad::start(0, data::Value((uint32_t)address, VALUE_TYPE_IP_ADDRESS), options, (void (*)(float))onAddressSet);
}

void SysSettingsEthernetStaticPage::editIpAddress() {
    edit(m_ipAddress);
}

void SysSettingsEthernetStaticPage::editDns() {
    edit(m_dns);
}

void SysSettingsEthernetStaticPage::editGateway() {
    edit(m_gateway);
}

void SysSettingsEthernetStaticPage::editSubnetMask() {
    edit(m_subnetMask);
}

int SysSettingsEthernetStaticPage::getDirty() {
    return
        m_ipAddressOrig != m_ipAddress ||
        m_dnsOrig != m_dns ||
        m_gatewayOrig != m_gateway ||
        m_subnetMaskOrig != m_subnetMask;
}

void SysSettingsEthernetStaticPage::set() {
    if (getDirty()) {
        SysSettingsEthernetPage *page = (SysSettingsEthernetPage *)getPreviousPage();

        page->m_ipAddress = m_ipAddress;
        page->m_dns = m_dns;
        page->m_gateway = m_gateway;
        page->m_subnetMask = m_subnetMask;

        popPage();
    }
}

#endif


////////////////////////////////////////////////////////////////////////////////

data::Value SysSettingsProtectionsPage::getData(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SYS_OUTPUT_PROTECTION_COUPLED) {
        return data::Value(persist_conf::isOutputProtectionCoupleEnabled() ? 1 : 0);
    }

    if (id == DATA_ID_SYS_SHUTDOWN_WHEN_PROTECTION_TRIPPED) {
        return data::Value(persist_conf::isShutdownWhenProtectionTrippedEnabled() ? 1 : 0);
    }

    if (id == DATA_ID_SYS_FORCE_DISABLING_ALL_OUTPUTS_ON_POWER_UP) {
        return data::Value(persist_conf::isForceDisablingAllOutputsOnPowerUpEnabled() ? 1 : 0);
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

	origLevel = level = data::Value(temperature::sensors[temp_sensor::AUX].prot_conf.level, VALUE_TYPE_FLOAT_CELSIUS);
	minLevel = OTP_AUX_MIN_LEVEL;
	maxLevel = OTP_AUX_MAX_LEVEL;
	defLevel = OTP_AUX_DEFAULT_LEVEL;

	origDelay = delay = data::Value(temperature::sensors[temp_sensor::AUX].prot_conf.delay, VALUE_TYPE_FLOAT_SECOND);
	minDelay = OTP_AUX_MIN_DELAY;
	maxDelay = OTP_AUX_MAX_DELAY;
	defaultDelay = OTP_CH_DEFAULT_DELAY;
}

data::Value SysSettingsAuxOtpPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_SYS_TEMP_AUX_OTP_STATE) {
		return state;
	}

    if (id == DATA_ID_SYS_TEMP_AUX_OTP_LEVEL) {
		return level;
	}

	if (id == DATA_ID_SYS_TEMP_AUX_OTP_DELAY) {
		return delay;
	}

	if (id == DATA_ID_SYS_TEMP_AUX_OTP_IS_TRIPPED) {
		return temperature::sensors[temp_sensor::AUX].isTripped() ? 1 : 0;
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

void SysSettingsAuxOtpPage::onDelaySet(float value) {
	popPage();
	SysSettingsAuxOtpPage *page = (SysSettingsAuxOtpPage *)getActivePage();
	page->delay = data::Value(value, page->delay.getType());
}

void SysSettingsAuxOtpPage::editDelay() {
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

////////////////////////////////////////////////////////////////////////////////

data::Value SysSettingsSoundPage::getData(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SYS_SOUND_IS_ENABLED) {
        return data::Value(persist_conf::isSoundEnabled() ? 1 : 0);
    }

    if (id == DATA_ID_SYS_SOUND_IS_CLICK_ENABLED) {
        return data::Value(persist_conf::isClickSoundEnabled() ? 1 : 0);
    }

    return data::Value();
}

void SysSettingsSoundPage::toggleSound() {
    if (persist_conf::isSoundEnabled()) {
        if (persist_conf::enableSound(false)) {
            infoMessageP(PSTR("Sound disabled!"));
        }
    } else {
        if (persist_conf::enableSound(true)) {
            infoMessageP(PSTR("Sound enabled!"));
        }
    }
}

void SysSettingsSoundPage::toggleClickSound() {
    if (persist_conf::isClickSoundEnabled()) {
        if (persist_conf::enableClickSound(false)) {
            infoMessageP(PSTR("Click sound disabled!"));
        }
    } else {
        if (persist_conf::enableClickSound(true)) {
            infoMessageP(PSTR("Click sound enabled!"));
        }
    }
}

#if OPTION_ENCODER

////////////////////////////////////////////////////////////////////////////////

SysSettingsEncoderPage::SysSettingsEncoderPage() {
    origConfirmationMode = confirmationMode = persist_conf::devConf2.flags.encoderConfirmationMode;
    origMovingSpeedDown = movingSpeedDown = persist_conf::devConf2.encoderMovingSpeedDown;
    origMovingSpeedUp = movingSpeedUp = persist_conf::devConf2.encoderMovingSpeedUp;
}

data::Value SysSettingsEncoderPage::getMin(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SYS_ENCODER_MOVING_DOWN_SPEED || id == DATA_ID_SYS_ENCODER_MOVING_UP_SPEED) {
        return encoder::MIN_MOVING_SPEED;
    }

    return data::Value();
}

data::Value SysSettingsEncoderPage::getMax(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SYS_ENCODER_MOVING_DOWN_SPEED || id == DATA_ID_SYS_ENCODER_MOVING_UP_SPEED) {
        return encoder::MAX_MOVING_SPEED;
    }

    return data::Value();
}

data::Value SysSettingsEncoderPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

    if (id == DATA_ID_SYS_ENCODER_CONFIRMATION_MODE) {
        return data::Value((int)confirmationMode);
    }

    if (id == DATA_ID_SYS_ENCODER_MOVING_DOWN_SPEED) {
        return data::Value((int)movingSpeedDown);
    }

    if (id == DATA_ID_SYS_ENCODER_MOVING_UP_SPEED) {
        return data::Value((int)movingSpeedUp);
    }

    return data::Value();
}

bool SysSettingsEncoderPage::setData(const data::Cursor &cursor, uint8_t id, data::Value value) {
    if (id == DATA_ID_SYS_ENCODER_MOVING_DOWN_SPEED) {
        movingSpeedDown = (uint8_t)value.getInt();
        return true;
    }

    if (id == DATA_ID_SYS_ENCODER_MOVING_UP_SPEED) {
        movingSpeedUp = (uint8_t)value.getInt();
        return true;
    }

    return false;
}

void SysSettingsEncoderPage::toggleConfirmationMode() {
    confirmationMode = confirmationMode ? 0 : 1;
}

int SysSettingsEncoderPage::getDirty() {
    return origConfirmationMode != confirmationMode || origMovingSpeedDown != movingSpeedDown || origMovingSpeedUp != movingSpeedUp;
}

void SysSettingsEncoderPage::set() {
	if (getDirty()) {
        persist_conf::setEncoderSettings(confirmationMode, movingSpeedDown, movingSpeedUp);
		infoMessageP(PSTR("Encoder settings changed!"), popPage);
	}
}

#endif

////////////////////////////////////////////////////////////////////////////////

data::Value SysSettingsDisplayPage::getMin(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SYS_DISPLAY_BRIGHTNESS) {
        return DISPLAY_BRIGHTNESS_MIN;
    }

    return data::Value();
}

data::Value SysSettingsDisplayPage::getMax(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SYS_DISPLAY_BRIGHTNESS) {
        return DISPLAY_BRIGHTNESS_MAX;
    }

    return data::Value();
}

data::Value SysSettingsDisplayPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = Page::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

    if (id == DATA_ID_SYS_DISPLAY_BRIGHTNESS) {
        return data::Value(persist_conf::devConf2.displayBrightness);
    }

    return data::Value();
}

bool SysSettingsDisplayPage::setData(const data::Cursor &cursor, uint8_t id, data::Value value) {
    if (id == DATA_ID_SYS_DISPLAY_BRIGHTNESS) {
        persist_conf::setDisplayBrightness((uint8_t)value.getInt());
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////

SysSettingsTriggerPage::SysSettingsTriggerPage() {
    m_sourceOrig = m_source = trigger::getSource();
    m_delayOrig = m_delay = trigger::getDelay();
    m_initiateContinuouslyOrig = m_initiateContinuously = trigger::isContinuousInitializationEnabled();
}

data::Value SysSettingsTriggerPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_TRIGGER_SOURCE) {
		return data::Value(m_source, data::ENUM_DEFINITION_TRIGGER_SOURCE);
	}

	if (id == DATA_ID_TRIGGER_DELAY) {
		return data::Value(m_delay, VALUE_TYPE_FLOAT_SECOND);
	}

	if (id == DATA_ID_TRIGGER_INITIATE_CONTINUOUSLY) {
        return data::Value(m_initiateContinuously ? 1 : 0);
	}

    return data::Value();
}

void SysSettingsTriggerPage::onTriggerSourceSet(uint8_t value) {
    popPage();
	SysSettingsTriggerPage *page = (SysSettingsTriggerPage*)getActivePage();
    page->m_source = (trigger::Source)value;
}

void SysSettingsTriggerPage::selectSource() {
    pushSelectFromEnumPage(data::g_triggerSourceEnumDefinition, m_source, 0, onTriggerSourceSet);
}

void SysSettingsTriggerPage::onDelaySet(float value) {
    popPage();
	SysSettingsTriggerPage *page = (SysSettingsTriggerPage*)getActivePage();
    page->m_delay = value;
}

void SysSettingsTriggerPage::editDelay() {
    NumericKeypadOptions options;

    options.editUnit = VALUE_TYPE_FLOAT_SECOND;

    options.def = 0;
	options.min = 0;
	options.max = 3600;

	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, data::Value(trigger::getDelay(), VALUE_TYPE_FLOAT_SECOND), options, onDelaySet);
}

void SysSettingsTriggerPage::toggleInitiateContinuously() {
    m_initiateContinuously = !m_initiateContinuously;
}

int SysSettingsTriggerPage::getDirty() {
    return m_sourceOrig != m_source || m_delayOrig != m_delay || m_initiateContinuouslyOrig != m_initiateContinuously;
}

void SysSettingsTriggerPage::set() {
    if (getDirty()) {
        trigger::setSource(m_source);
        trigger::setDelay(m_delay);
        trigger::enableInitiateContinuous(m_initiateContinuously);

        if (m_source == trigger::SOURCE_PIN1) {
            persist_conf::devConf2.ioPins[0].function = io_pins::FUNCTION_TINPUT;
        }

        persist_conf::saveDevice2();

        infoMessageP(PSTR("Trigger settings saved!"), popPage);
    }
}


////////////////////////////////////////////////////////////////////////////////

SysSettingsIOPinsPage::SysSettingsIOPinsPage() {
    for (int i = 0; i < 3; ++i) {
        m_polarityOrig[i] = m_polarity[i] = (io_pins::Polarity) persist_conf::devConf2.ioPins[i].polarity;
        m_functionOrig[i] = m_function[i] = (io_pins::Function) persist_conf::devConf2.ioPins[i].function;
    }
}

data::Value SysSettingsIOPinsPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

    if (id == DATA_ID_IO_PIN_NUMBER) {
		return data::Value(cursor.i + 1);
	}

	if (id == DATA_ID_IO_PIN_POLARITY) {
		return data::Value(m_polarity[cursor.i], data::ENUM_DEFINITION_IO_PINS_POLARITY);
	}

	if (id == DATA_ID_IO_PIN_FUNCTION) {
        if (cursor.i == 0) {
    		return data::Value(m_function[cursor.i], data::ENUM_DEFINITION_IO_PINS_INPUT_FUNCTION);
        } else {
    		return data::Value(m_function[cursor.i], data::ENUM_DEFINITION_IO_PINS_OUTPUT_FUNCTION);
        }
	}

    return data::Value();
}

void SysSettingsIOPinsPage::togglePolarity() {
    int i = g_foundWidgetAtDown.cursor.i;
    m_polarity[i] = m_polarity[i] == io_pins::POLARITY_NEGATIVE ? io_pins::POLARITY_POSITIVE : io_pins::POLARITY_NEGATIVE;
}

void SysSettingsIOPinsPage::onFunctionSet(uint8_t value) {
    popPage();
	SysSettingsIOPinsPage *page = (SysSettingsIOPinsPage*)getActivePage();
    page->m_function[page->pinNumber] = (io_pins::Function)value;
}

void SysSettingsIOPinsPage::selectFunction() {
    pinNumber = g_foundWidgetAtDown.cursor.i;
    pushSelectFromEnumPage(pinNumber == 0 ? data::g_ioPinsInputFunctionEnumDefinition : data::g_ioPinsOutputFunctionEnumDefinition,
        m_function[pinNumber], 0, onFunctionSet);
}

int SysSettingsIOPinsPage::getDirty() {
    for (int i = 0; i < 3; ++i) {
        if (m_polarityOrig[i] != m_polarity[i] || m_functionOrig[i] != m_function[i]) {
            return true;
        }
    }
    return false;
}

void SysSettingsIOPinsPage::set() {
    if (getDirty()) {
        for (int i = 0; i < 3; ++i) {
            persist_conf::devConf2.ioPins[i].polarity = m_polarity[i];
            persist_conf::devConf2.ioPins[i].function = m_function[i];

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
            if (i > 0 && m_function[i] == io_pins::FUNCTION_NONE) {
                digitalWrite(i == 1 ? DOUT : DOUT2, 0);
            }
#endif
        }

        if (persist_conf::saveDevice2()) {
            infoMessageP(PSTR("Digital I/O pin settings saved!"), popPage);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////

SysSettingsSerialPage::SysSettingsSerialPage() {
    m_enabledOrig = m_enabled = persist_conf::isSerialEnabled();
    m_baudIndexOrig = m_baudIndex = persist_conf::getSerialBaudIndex();
    m_parityOrig = m_parity = (serial::Parity)persist_conf::getSerialParity();
}

data::Value SysSettingsSerialPage::getMin(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SERIAL_BAUD) {
        return 1;
    }

    return data::Value();
}

data::Value SysSettingsSerialPage::getMax(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SERIAL_BAUD) {
        return serial::g_baudsSize;
    }

    return data::Value();
}

data::Value SysSettingsSerialPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != VALUE_TYPE_NONE) {
		return value;
	}

    if (id == DATA_ID_SERIAL_ENABLED) {
        return data::Value(m_enabled ? 1 : 0);
    }

    if (id == DATA_ID_SERIAL_BAUD) {
		return data::Value((int)m_baudIndex, VALUE_TYPE_SERIAL_BAUD_INDEX);
	}

	if (id == DATA_ID_SERIAL_PARITY) {
		return data::Value(m_parity, data::ENUM_DEFINITION_SERIAL_PARITY);
	}

    return data::Value();
}

bool SysSettingsSerialPage::setData(const data::Cursor &cursor, uint8_t id, data::Value value) {
    if (id == DATA_ID_SERIAL_BAUD) {
        m_baudIndex = (uint8_t)value.getInt();
        return true;
    }

    return false;
}

void SysSettingsSerialPage::toggle() {
    m_enabled = !m_enabled;
}

void SysSettingsSerialPage::onParitySet(uint8_t value) {
    popPage();
	SysSettingsSerialPage *page = (SysSettingsSerialPage*)getActivePage();
    page->m_parity = (serial::Parity)value;
}

void SysSettingsSerialPage::selectParity() {
    pushSelectFromEnumPage(data::g_serialParityEnumDefinition, m_parity, 0, onParitySet);
}

int SysSettingsSerialPage::getDirty() {
    return m_enabledOrig != m_enabled || m_baudIndexOrig != m_baudIndex || m_parityOrig != m_parity;
}

void SysSettingsSerialPage::set() {
    if (getDirty()) {
        if (persist_conf::setSerialSettings(m_enabled, m_baudIndex, m_parity)) {
            infoMessageP(PSTR("Serial settings saved!"), popPage);
        }
    }
}

}
}
} // namespace eez::psu::gui

#endif
