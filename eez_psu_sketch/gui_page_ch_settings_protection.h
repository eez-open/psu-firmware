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

namespace eez {
namespace psu {
namespace gui {

class ChSettingsProtectionPage : public Page {
public:
	static void clear();
	static void clearAndDisable();

	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);
};

class ChSettingsProtectionSetPage : public SetPage {
public:
	void takeSnapshot(data::Snapshot *snapshot);
	data::Value getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot);

	int getDirty();
	void set();

	void toggleState();
	void editLimit();
	void editLevel();
	void editDelay();

protected:
	int origState;
	int state;

	data::Value origLimit;
	data::Value limit;
	float minLimit;
	float maxLimit;
	float defLimit;

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

	virtual void setParams(bool checkLoad) = 0;

	static void onLimitSet(float value);
	static void onLevelSet(float value);
	static void onDelaySet(float value);
	static void onSetFinish();
};

class ChSettingsOvpProtectionPage : public ChSettingsProtectionSetPage {
public:
	ChSettingsOvpProtectionPage();

protected:
	void setParams(bool checkLoad);

private:
	static void onSetParamsOk();
};

class ChSettingsOcpProtectionPage : public ChSettingsProtectionSetPage {
public:
	ChSettingsOcpProtectionPage();

protected:
	void setParams(bool checkLoad);

private:
	static void onSetParamsOk();
};

class ChSettingsOppProtectionPage : public ChSettingsProtectionSetPage {
public:
	ChSettingsOppProtectionPage();

protected:
	void setParams(bool checkLoad);

private:
	static void onSetParamsOk();
};

class ChSettingsOtpProtectionPage : public ChSettingsProtectionSetPage {
public:
	ChSettingsOtpProtectionPage();

protected:
	void setParams(bool checkLoad);
};

}
}
} // namespace eez::psu::gui
