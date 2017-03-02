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

class ChSettingsAdvPage : public Page {
public:
	data::Value getData(const data::Cursor &cursor, uint8_t id);
};

class ChSettingsAdvLRipplePage : public SetPage {
public:
	ChSettingsAdvLRipplePage();

	data::Value getData(const data::Cursor &cursor, uint8_t id);

	int getDirty();
	void set();

	void toggleAutoMode();
	void toggleStatus();

private:
	int origAutoMode;
	int autoMode;

    int origStatus;
	int status;
};

class ChSettingsAdvRSensePage : public Page {
public:
	data::Value getData(const data::Cursor &cursor, uint8_t id);

	void toggleStatus();
};

class ChSettingsAdvRProgPage : public Page {
public:
	void toggleStatus();
};

class ChSettingsAdvCouplingPage : public Page {
public:
	data::Value getData(const data::Cursor &cursor, uint8_t id);

    void uncouple();
    void setParallelInfo();
    void setSeriesInfo();
    void setParallel();
    void setSeries();

private:
    static int selectedMode;
};

class ChSettingsAdvTrackingPage : public Page {
public:
    void toggleTrackingMode();
};

class ChSettingsAdvViewPage : public SetPage {
public:
	ChSettingsAdvViewPage();

	data::Value getData(const data::Cursor &cursor, uint8_t id);

    void editDisplayValue1();
    void editDisplayValue2();
    void swapDisplayValues();
    void editYTViewRate();

	int getDirty();
	void set();

private:
	uint8_t origDisplayValue1;
	uint8_t displayValue1;

	uint8_t origDisplayValue2;
	uint8_t displayValue2;

	float origYTViewRate;
	float ytViewRate;

    static void onDisplayValue1Set(uint8_t value);
    static void onDisplayValue2Set(uint8_t value);
    static void onYTViewRateSet(float value);
};

}
}
} // namespace eez::psu::gui
