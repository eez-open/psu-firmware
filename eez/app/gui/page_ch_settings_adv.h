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

#include "eez/mw/gui/page.h"

namespace eez {
namespace app {
namespace gui {

class ChSettingsAdvLRipplePage : public SetPage {
public:
	ChSettingsAdvLRipplePage();

	int getDirty();
	void set();

	void toggleAutoMode();
	void toggleStatus();

	int autoMode;
	int status;

private:
	int origAutoMode;
    int origStatus;
};

class ChSettingsAdvRemotePage : public Page {
public:
	void toggleSense();
    void toggleProgramming();
};

class ChSettingsAdvRangesPage : public Page {
public:
	void selectMode();
    void toggleAutoRanging();

private:
    static void onModeSet(uint8_t value);
};

class ChSettingsAdvCouplingPage : public Page {
public:
    void uncouple();
    void setParallelInfo();
    void setSeriesInfo();
    void setParallel();
    void setSeries();

    static int selectedMode;
};

class ChSettingsAdvTrackingPage : public Page {
public:
    void toggleTrackingMode();
};

class ChSettingsAdvViewPage : public SetPage {
public:
	ChSettingsAdvViewPage();

    void editDisplayValue1();
    void editDisplayValue2();
    void swapDisplayValues();
    void editYTViewRate();

	int getDirty();
	void set();

	uint8_t displayValue1;
	uint8_t displayValue2;
	float ytViewRate;

private:
	uint8_t origDisplayValue1;
	uint8_t origDisplayValue2;
	float origYTViewRate;

    static bool isDisabledDisplayValue1(uint8_t value);
    static void onDisplayValue1Set(uint8_t value);
    static bool isDisabledDisplayValue2(uint8_t value);
    static void onDisplayValue2Set(uint8_t value);
    static void onYTViewRateSet(float value);
};

}
}
} // namespace eez::app::gui
