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

namespace eez {
namespace app {
namespace simulator {
namespace arduino {

/// Fake (do nothing) implementation of Enc28J60Network class
class Enc28J60Network {
public:
    static void setControlCS(int pin);
};

/// Arduino Ethernet object simulator
class SimulatorEthernet {
public:
    bool begin(uint8_t *mac, uint8_t *ipAddress = 0, uint8_t *dns = 0, uint8_t *gateway = 0, uint8_t *subnetMask = 0);
    uint8_t maintain();

    IPAddress localIP();
    IPAddress subnetMask();
    IPAddress gatewayIP();
    IPAddress dnsServerIP();
};

extern SimulatorEthernet Ethernet;

}
}
}
} // namespace eez::app::simulator::arduino;

using namespace eez::app::simulator::arduino;
