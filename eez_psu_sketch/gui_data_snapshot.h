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

#include "gui_data.h"
#include "gui_keypad.h"
#include "gui_edit_mode_snapshot.h"

namespace eez {
namespace psu {
namespace gui {
namespace data {

struct ChannelSnapshotFlags {
    unsigned ok : 1;
    unsigned mode : 1;
    unsigned state : 1;
	unsigned lrip : 1;
    unsigned ovp : 2;
    unsigned ocp : 2;
    unsigned opp : 2;
    unsigned otp_ch : 2;
    unsigned dp : 2;
};

struct ChannelSnapshot {
    Value mon_value;
    float u_set;
    float i_set;
    ChannelSnapshotFlags flags;
};

struct SnapshotFlags {
    unsigned otp : 2;
};

struct Snapshot {
    ChannelSnapshot channelSnapshots[CH_NUM];
	keypad::Snapshot keypadSnapshot;
    edit_mode::Snapshot editModeSnapshot;
    Value alertMessage;
	SnapshotFlags flags;
	char *selfTestResult;

    unsigned long lastSnapshotTime;

    void takeSnapshot();

    Value get(const Cursor &cursor, uint8_t id);
    bool isBlinking(const Cursor &cursor, uint8_t id);
};

extern Snapshot currentSnapshot;
extern Snapshot previousSnapshot;

}
}
}
} // namespace eez::psu::ui::data
