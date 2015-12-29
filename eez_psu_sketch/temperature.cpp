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
 
#include "psu.h"
#include "serial_psu.h"
#include "ethernet.h"
#include "persist_conf.h"
#include "sound.h"

namespace eez {
namespace psu {

using namespace scpi;

namespace temperature {

ProtectionConfiguration prot_conf[temp_sensor::COUNT];

static unsigned long sensor_temperature_last_measured_tick[temp_sensor::COUNT];
static float sensor_temperature[temp_sensor::COUNT];
static bool sensor_otp_alarmed[temp_sensor::COUNT];
static unsigned long sensor_otp_alarmed_started_tick[temp_sensor::COUNT];
static bool sensor_otp_tripped[temp_sensor::COUNT];

////////////////////////////////////////////////////////////////////////////////

static void set_otp_reg(temp_sensor::Type sensor, bool on) {
    if (sensor == temp_sensor::MAIN) {
        psu::setQuesBits(QUES_TEMP, on);
    }
    else {
        int bit_mask = reg_get_ques_isum_bit_mask_for_channel_protection_value(sensor);
        Channel::get(sensor == temp_sensor::S1 || sensor == temp_sensor::BAT1 ? 0 : 1).setQuesBits(bit_mask, on);
    }
}

static void sensor_protection_enter(temp_sensor::Type sensor) {
    sensor_otp_tripped[sensor] = true;

    if (sensor == temp_sensor::MAIN) {
        for (int i = 0; i < CH_NUM; ++i) {
            Channel::get(i).outputEnable(false);
        }

        psu::powerDownBySensor();
    }
    else {
        Channel::get(sensor == temp_sensor::S1 || sensor == temp_sensor::BAT1 ? 0 : 1).outputEnable(false);
    }

    set_otp_reg(sensor, true);

    sound::playBeep();
}

static void sensor_protection_check(unsigned long tick_usec, temp_sensor::Type sensor) {
    if (sensor_otp_tripped[sensor]) {
        return;
    }

    if (prot_conf[sensor].state && sensor_temperature[sensor] >= prot_conf[sensor].level) {
        float delay = prot_conf[sensor].delay;
        if (delay > 0) {
            if (sensor_otp_alarmed[sensor]) {
                if (tick_usec - sensor_otp_alarmed_started_tick[sensor] >= delay * 1000000UL) {
                    sensor_otp_alarmed[sensor] = 0;
                    sensor_protection_enter(sensor);
                }
            }
            else {
                sensor_otp_alarmed[sensor] = 1;
                sensor_otp_alarmed_started_tick[sensor] = tick_usec;
            }
        }
        else {
            sensor_protection_enter(sensor);
        }
    }
    else {
        sensor_otp_alarmed[sensor] = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////

void tick(unsigned long tick_usec) {
    if (tick_usec - sensor_temperature_last_measured_tick[temp_sensor::MAIN] >= TEMP_SENSOR_READ_EVERY_MS * 1000L) {
        sensor_temperature_last_measured_tick[temp_sensor::MAIN] = tick_usec;
        sensor_temperature[temp_sensor::MAIN] = temp_sensor::read(temp_sensor::MAIN);
        sensor_protection_check(tick_usec, temp_sensor::MAIN);
    }
}

float measure(temp_sensor::Type sensor) {
    sensor_temperature[sensor] = temp_sensor::read(sensor);
    return sensor_temperature[sensor];
}

void clearProtection(temp_sensor::Type sensor) {
    sensor_otp_tripped[sensor] = false;
    set_otp_reg(sensor, false);
}

bool isSensorTripped(temp_sensor::Type sensor) {
    return sensor_otp_tripped[sensor];
}

bool isChannelTripped(Channel *channel) {
    if (sensor_otp_tripped[temp_sensor::MAIN])
        return true;

    if (channel->index == 1) {
        if (sensor_otp_tripped[temp_sensor::S1] || sensor_otp_tripped[temp_sensor::BAT1])
            return true;
    }
    else {
        if (sensor_otp_tripped[temp_sensor::S2] || sensor_otp_tripped[temp_sensor::BAT2])
            return true;

    }

    return false;
}

}
}
} // namespace eez::psu::temperature