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
#include "gui_page_sys_settings_protections.h"

namespace eez {
namespace psu {
namespace gui {

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

}
}
} // namespace eez::psu::gui
