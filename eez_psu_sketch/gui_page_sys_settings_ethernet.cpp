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

#include "ethernet.h"
#include "persist_conf.h"

#include "gui_data_snapshot.h"
#include "gui_page_sys_settings_ethernet.h"

namespace eez {
namespace psu {
namespace gui {

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

void SysSettingsEthernetPage::enable() {
    if (!persist_conf::isEthernetEnabled()) {
        persist_conf::enableEthernet(true);
    }
}

void SysSettingsEthernetPage::disable() {
    if (persist_conf::isEthernetEnabled()) {
        persist_conf::enableEthernet(false);
    }
}


}
}
} // namespace eez::psu::gui
