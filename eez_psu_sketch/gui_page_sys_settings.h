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

namespace eez {
namespace psu {
namespace gui {

class SysSettingsDateTimePage : public SetPage {
public:
	SysSettingsDateTimePage();

	void takeSnapshot(data::Snapshot *snapshot);
	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);

	void edit();
	void setValue(float value);
	int getDirty();
	void set();

	void toggleDst();

private:
	datetime::DateTime origDateTime;
	datetime::DateTime dateTime;

	int16_t origTimeZone;
	int16_t timeZone;

	unsigned int origDst;
	unsigned int dst;
};

class SysSettingsEthernetPage : public Page {
public:
	void takeSnapshot(data::Snapshot *snapshot);
	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);

    static void enable();
    static void disable();
};

class SysSettingsProtectionsPage : public Page {
public:
	void takeSnapshot(data::Snapshot *snapshot);
	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);

    static void toggleOutputProtectionCouple();
    static void toggleShutdownWhenProtectionTripped();
    static void toggleForceDisablingAllOutputsOnPowerUp();
};

class SysSettingsAuxOtpPage : public SetPage {
public:
    SysSettingsAuxOtpPage();

	void takeSnapshot(data::Snapshot *snapshot);
	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);

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
	void takeSnapshot(data::Snapshot *snapshot);
	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);

    static void toggleSound();
    static void toggleClickSound();
};

#if OPTION_ENCODER

class SysSettingsEncoderPage : public SetPage {
public:
    SysSettingsEncoderPage();

	void takeSnapshot(data::Snapshot *snapshot);
	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);
    data::Value getMin(const data::Cursor &cursor, uint8_t id);
    data::Value getMax(const data::Cursor &cursor, uint8_t id);
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

}
}
} // namespace eez::psu::gui
