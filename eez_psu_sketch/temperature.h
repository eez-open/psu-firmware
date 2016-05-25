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

#include "temp_sensor.h"

namespace eez {
namespace psu {

class Channel;

/// Temperature measurement and protection.
namespace temperature {


/// Configuration data for the temperature protection.
struct ProtectionConfiguration {
    temp_sensor::Type sensor;
    float delay;
    float level;
    bool state;
};

extern ProtectionConfiguration prot_conf[temp_sensor::COUNT];

void tick(unsigned long tick_usec);

float measure(temp_sensor::Type sensor);

void clearProtection(temp_sensor::Type sensor);
bool isSensorTripped(temp_sensor::Type sensor);

bool isChannelTripped(Channel *channel);

}
}
} // namespace eez::psu::temperature
