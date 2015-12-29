/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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
namespace psu {
namespace simulator {
namespace arduino {

/// Bare minimum implementation of the Arduino EthernetClient class
class EthernetClient {
public:
    EthernetClient();
    EthernetClient(bool valid);

    operator bool();
    bool operator==(EthernetClient &other) { return true; }

    bool connected();

    size_t available();
    size_t read(uint8_t*, size_t);
    size_t write(const char *data, size_t len);

    void stop();

private:
    bool valid;
};

}
}
}
} // namespace eez::psu::simulator::arduino;

using namespace eez::psu::simulator::arduino;
