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

#include "psu.h"
#include "board.h"
#if (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12) && OPTION_WATCHDOG
#include "watchdog.h"
#endif

namespace eez {
namespace app {
namespace board {

void powerUp() {
    digitalWrite(PWR_SSTART, HIGH);
    delay(700);

    DebugTrace("PWR_DIRECT -> HIGH");
    digitalWrite(PWR_DIRECT, HIGH);
    delay(100);

    digitalWrite(PWR_SSTART, LOW);
}

void powerDown() {
//#if (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12) && OPTION_WATCHDOG && (CONF_DEBUG || CONF_DEBUG_LATEST)
//    watchdog::printInfo();
//#endif

    DebugTrace("PWR_DIRECT -> LOW");
    digitalWrite(PWR_DIRECT, LOW);
}

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9

void cvLedSwitch(Channel *channel, bool on) {
    digitalWrite(channel->cv_led_pin, on);
}

void ccLedSwitch(Channel *channel, bool on) {
    digitalWrite(channel->cc_led_pin, on);
}

#endif

}
}
} // namespace eez::app::board