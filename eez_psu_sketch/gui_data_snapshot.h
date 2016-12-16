/*
* EEZ PSU Firmware
* Copyright (C) 2015-present, Envox d.o.o.
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

#include "event_queue.h"
#include "gui_data.h"
#include "gui_keypad.h"
#include "gui_edit_mode_snapshot.h"

namespace eez {
namespace psu {
namespace gui {
namespace data {

struct ChannelSnapshotFlags {
    unsigned status : 2;
    unsigned state : 1;
    unsigned mode : 1;
	unsigned lrip : 1;
	unsigned rprog: 1;
    unsigned ovp : 2;
    unsigned ocp : 2;
    unsigned opp : 2;
    unsigned otp_ch : 2;
    unsigned dp : 2;
	unsigned cal_enabled: 1;
	unsigned temperatureStatus: 2;
};

struct ChannelSnapshot {
    Value mon_value;
    float u_set;
	float u_mon;
	float u_monDac;
	float u_limit;
    float i_set;
	float i_mon;
	float i_monDac;
	float i_limit;
	float p_mon;
    ChannelSnapshotFlags flags;
	float temperature;
	uint32_t onTimeTotal;
	uint32_t onTimeLast;
};

struct ProfileChannelSnapshot {
	unsigned outputStatus: 1;
	float u_set;
	float i_set;
};

struct ProfileSnapshot {
	unsigned status: 1;
	unsigned isAutoRecallLocation: 1;
	char remark[PROFILE_NAME_MAX_LENGTH + 1];
	ProfileChannelSnapshot channels[CH_MAX];
};

struct SnapshotFlags {
    unsigned otp : 2;
	unsigned auxTemperatureStatus: 2;
	unsigned fanStatus: 2;
	unsigned setPageDirty: 1;
	unsigned switch1: 1;
	unsigned switch2: 1;
    unsigned switch3: 1;
	unsigned channelDisplayedValues: 3;
    unsigned channelCouplingMode: 2;
    unsigned isVoltageBalanced: 1;
    unsigned isCurrentBalanced: 1;
};

struct Snapshot {
    ChannelSnapshot channelSnapshots[CH_MAX];
	keypad::Snapshot keypadSnapshot;
    edit_mode::Snapshot editModeSnapshot;
    Value alertMessage;
    Value alertMessage2;
	SnapshotFlags flags;
	event_queue::Event lastEvent;
	event_queue::Event events[event_queue::EVENTS_PER_PAGE];
	Value eventQueuePageInfo;
	uint32_t onTimeTotal;
	uint32_t onTimeLast;
	float auxTemperature;
	float fanSpeed;
	ProfileSnapshot profile;

    unsigned long lastSnapshotTime;

    void takeSnapshot();

    Value get(const Cursor &cursor, uint8_t id);
    Value getEditValue(const Cursor &cursor, uint8_t id);
    bool isBlinking(const Cursor &cursor, uint8_t id);
};

extern Snapshot currentSnapshot;
extern Snapshot previousSnapshot;

}
}
}
} // namespace eez::psu::ui::data
