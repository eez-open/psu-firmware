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
 
#pragma once

#include "gui_page.h"

#include "datetime.h"
#include "trigger.h"
#include "io_pins.h"
#include "serial_psu.h"

namespace eez {
namespace psu {
namespace gui {

class SysSettingsDateTimePage : public SetPage {
public:
	SysSettingsDateTimePage();

	data::Value getData(const data::Cursor &cursor, uint8_t id);

	void edit();
	void setValue(float value);
	int getDirty();
	void set();

    void toggleNtp();
    void editNtpServer();
	void toggleDst();

private:
    bool origNtpEnabled;
    bool ntpEnabled;

    char origNtpServer[32+1];
    char ntpServer[32+1];

	datetime::DateTime origDateTime;
	datetime::DateTime dateTime;

	int16_t origTimeZone;
	int16_t timeZone;

	bool origDst;
	bool dst;

#if OPTION_ETHERNET
    enum {
        TEST_NTP_SERVER_OPERATION_STATE_INIT = -1,
        TEST_NTP_SERVER_OPERATION_STATE_STARTED = -2
    };

    static void onSetNtpServer(char * value);

    static void checkTestNtpServerStatus();
    void testNtpServer();
#endif

    void doSet();
};

#if OPTION_ETHERNET

class SysSettingsEthernetPage : public SetPage {
    friend class SysSettingsEthernetStaticPage;

public:
    SysSettingsEthernetPage();

	data::Value getData(const data::Cursor &cursor, uint8_t id);

    void toggle();
    void toggleDhcp();
    void editStaticAddress();
    void editScpiPort();

	int getDirty();
	void set();

private:
    bool m_enabledOrig;
    bool m_enabled;

    bool m_dhcpEnabledOrig;
    bool m_dhcpEnabled;

    uint32_t m_ipAddressOrig;
    uint32_t m_ipAddress;

    uint32_t m_dnsOrig;
    uint32_t m_dns;

    uint32_t m_gatewayOrig;
    uint32_t m_gateway;

    uint32_t m_subnetMaskOrig;
    uint32_t m_subnetMask;

    uint16_t m_scpiPortOrig;
    uint16_t m_scpiPort;

    static void onScpiPortSet(float value);
};

class SysSettingsEthernetStaticPage : public SetPage {
public:
    SysSettingsEthernetStaticPage();

	data::Value getData(const data::Cursor &cursor, uint8_t id);

    void editIpAddress();
    void editDns();
    void editGateway();
    void editSubnetMask();

	int getDirty();
	void set();

private:
    enum Field {
        FIELD_IP_ADDRESS,
        FIELD_DNS,
        FIELD_GATEWAY,
        FIELD_SUBNET_MASK
    };

    uint32_t m_ipAddressOrig;
    uint32_t m_ipAddress;

    uint32_t m_dnsOrig;
    uint32_t m_dns;

    uint32_t m_gatewayOrig;
    uint32_t m_gateway;

    uint32_t m_subnetMaskOrig;
    uint32_t m_subnetMask;

    uint32_t *m_editAddress;

    void edit(uint32_t &address);
    static void onAddressSet(uint32_t address);
};

#endif

class SysSettingsProtectionsPage : public Page {
public:
	data::Value getData(const data::Cursor &cursor, uint8_t id);

    static void toggleOutputProtectionCouple();
    static void toggleShutdownWhenProtectionTripped();
    static void toggleForceDisablingAllOutputsOnPowerUp();
};

class SysSettingsAuxOtpPage : public SetPage {
public:
    SysSettingsAuxOtpPage();

	data::Value getData(const data::Cursor &cursor, uint8_t id);

	int getDirty();
	void set();

	void toggleState();
	void editLevel();
	void editDelay();

    static void clear();

protected:
	int origState;
	int state;

	data::Value origLevel;
	data::Value level;
	float minLevel;
	float maxLevel;
	float defLevel;

	data::Value origDelay;
	data::Value delay;
	float minDelay;
	float maxDelay;
	float defaultDelay;

	virtual void setParams();

	static void onLevelSet(float value);
	static void onDelaySet(float value);
};

class SysSettingsSoundPage : public Page {
public:
	data::Value getData(const data::Cursor &cursor, uint8_t id);

    static void toggleSound();
    static void toggleClickSound();
};

#if OPTION_ENCODER

class SysSettingsEncoderPage : public SetPage {
public:
    SysSettingsEncoderPage();

    data::Value getMin(const data::Cursor &cursor, uint8_t id);
    data::Value getMax(const data::Cursor &cursor, uint8_t id);
	data::Value getData(const data::Cursor &cursor, uint8_t id);
    bool setData(const data::Cursor &cursor, uint8_t id, data::Value value);

    void toggleConfirmationMode();

	int getDirty();
	void set();

private:
    uint8_t origConfirmationMode;
    uint8_t confirmationMode;

    uint8_t origMovingSpeedDown;
    uint8_t movingSpeedDown;

    uint8_t origMovingSpeedUp;
    uint8_t movingSpeedUp;
};

#endif

class SysSettingsDisplayPage : public Page {
public:
    data::Value getMin(const data::Cursor &cursor, uint8_t id);
    data::Value getMax(const data::Cursor &cursor, uint8_t id);
	data::Value getData(const data::Cursor &cursor, uint8_t id);
    bool setData(const data::Cursor &cursor, uint8_t id, data::Value value);
};

class SysSettingsTriggerPage : public SetPage {
public:
    SysSettingsTriggerPage();

    data::Value getData(const data::Cursor &cursor, uint8_t id);

    void selectSource();
    void editDelay();
    void toggleInitiateContinuously();
    
	int getDirty();
	void set();

private:
    trigger::Source m_sourceOrig;
    trigger::Source m_source;

    float m_delayOrig;
    float m_delay;

    bool m_initiateContinuouslyOrig;
    bool m_initiateContinuously;

    static void onTriggerSourceSet(uint8_t value);
    static void onDelaySet(float value);
};

class SysSettingsIOPinsPage : public SetPage {
public:
    SysSettingsIOPinsPage();

    data::Value getData(const data::Cursor &cursor, uint8_t id);

    void togglePolarity();
    void selectFunction();
    
	int getDirty();
	void set();

private:
    int pinNumber;

    io_pins::Polarity m_polarityOrig[3];
    io_pins::Polarity m_polarity[3];

    io_pins::Function m_functionOrig[3];
    io_pins::Function m_function[3];

    static void onFunctionSet(uint8_t value);
};

class SysSettingsSerialPage : public SetPage {
public:
    SysSettingsSerialPage();

    data::Value getMin(const data::Cursor &cursor, uint8_t id);
    data::Value getMax(const data::Cursor &cursor, uint8_t id);
    data::Value getData(const data::Cursor &cursor, uint8_t id);
    bool setData(const data::Cursor &cursor, uint8_t id, data::Value value);

    void toggle();
    void selectParity();
    
	int getDirty();
	void set();

private:
    bool m_enabledOrig;
    bool m_enabled;

    uint8_t m_baudIndexOrig;
    uint8_t m_baudIndex;

    serial::Parity m_parityOrig;
    serial::Parity m_parity;

    static void onParitySet(uint8_t value);
};

}
}
} // namespace eez::psu::gui
