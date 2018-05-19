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

#include "eez/app/psu.h"

#include "eez/app/platform/simulator/ethernet/UIPEthernet.h"
#include "eez/app/platform/simulator/ethernet/UIPServer.h"
#include "eez/app/platform/simulator/ethernet/UIPClient.h"
#include "eez/mw/platform/simulator/ethernet.h"

namespace eez {
namespace app {
namespace simulator {
namespace arduino {

////////////////////////////////////////////////////////////////////////////////

void Enc28J60Network::setControlCS(int pin) {
}

////////////////////////////////////////////////////////////////////////////////

SimulatorEthernet Ethernet;

bool SimulatorEthernet::begin(uint8_t *mac, uint8_t *, uint8_t *, uint8_t *, uint8_t *) {
    delay(1000);
    return true;
}

uint8_t SimulatorEthernet::maintain() {
    return 0;
}

IPAddress SimulatorEthernet::localIP() {
    return IPAddress();
}

IPAddress SimulatorEthernet::subnetMask() {
    return IPAddress();
}

IPAddress SimulatorEthernet::gatewayIP() {
    return IPAddress();
}

IPAddress SimulatorEthernet::dnsServerIP() {
    return IPAddress();
}

////////////////////////////////////////////////////////////////////////////////

EthernetServer::EthernetServer(int port_) : port(port_), client(true) {
}

void EthernetServer::begin() {
    bind_result = platform::ethernet::bind(port);
}

EthernetClient EthernetServer::available() {
    if (!bind_result) return EthernetClient();
    return client;
}

////////////////////////////////////////////////////////////////////////////////

EthernetClient::EthernetClient() : valid(false) {
}

EthernetClient::EthernetClient(bool valid_) : valid(valid_) {
}

bool EthernetClient::connected() {
    return platform::ethernet::connected();
}

EthernetClient::operator bool() {
    return valid && platform::ethernet::client_available();
}

size_t EthernetClient::available() {
    return platform::ethernet::available();
}

size_t EthernetClient::read(uint8_t* buffer, size_t buffer_size) {
    return platform::ethernet::read((char *)buffer, (int)buffer_size);
}

size_t EthernetClient::write(const char *buffer, size_t buffer_size) {
    return platform::ethernet::write(buffer, (int)buffer_size);
}

void EthernetClient::flush() {
}

void EthernetClient::stop() {
    platform::ethernet::stop();
}

}
}
}
} // namespace eez::app::simulator::arduino;
