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
#include "watchdog.h"
#include "timer.h"

#if (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12) && OPTION_WATCHDOG

namespace eez {
namespace app {
namespace watchdog {

static bool watchdogEnabled = false;
static Interval watchdogInterval(WATCHDOG_INTERVAL);
static uint32_t g_lastWatchdogImpulseTime[3];

static uint8_t g_watchdogState;

void enable() {
    watchdogEnabled = true;
    watchdogInterval.reset();
	pinMode(WATCHDOG, OUTPUT);
    g_watchdogState = HIGH;
    digitalWrite(WATCHDOG, g_watchdogState);
}

void disable() {
    watchdogEnabled = false;
	pinMode(WATCHDOG, INPUT);
}

void tick(uint32_t tick_usec) {
    if (watchdogEnabled) {
        if (!isPowerUp()) {
            // WATCHDOG pin impedance must be changed to be sure that TPS3705 will not detect prolonged delay in receiving heartbeat impulse
            // when waking up sequence from Stand-by mode is initiated and consequently switch off input power.
            // That could generate a "glitch" in powering SPI periperals on the Power board when huge load is connected.
            disable();
        }
    } else {
        if (isPowerUp()) {
            enable();
        }
    }

    if (watchdogEnabled) {
	    if (watchdogInterval.test(tick_usec)) {
#if CONF_DEBUG
		    if (debug::g_debugWatchdog) {
                g_lastWatchdogImpulseTime[0] = g_lastWatchdogImpulseTime[1];
                g_lastWatchdogImpulseTime[1] = g_lastWatchdogImpulseTime[2];
                g_lastWatchdogImpulseTime[2] = micros();
#endif
                g_watchdogState = g_watchdogState == HIGH ? LOW : HIGH;
		        digitalWrite(WATCHDOG, g_watchdogState);
#if CONF_DEBUG
		    }
#endif
	    }
    }
}

#if CONF_DEBUG || CONF_DEBUG_LATEST
void printInfo() {
    uint32_t now = micros();
    DebugTraceF("Last watchdog impulses: %lu, %u, %lu, %u, %lu, %u, %lu",
        watchdog::g_lastWatchdogImpulseTime[0], watchdog::g_lastWatchdogImpulseTime[1] - watchdog::g_lastWatchdogImpulseTime[0],
        watchdog::g_lastWatchdogImpulseTime[1], watchdog::g_lastWatchdogImpulseTime[2] - watchdog::g_lastWatchdogImpulseTime[1],
        watchdog::g_lastWatchdogImpulseTime[2], now - watchdog::g_lastWatchdogImpulseTime[2],
        now);
}
#endif

}
}
} // namespace eez::app::watchdog

#endif