/*
 * EEZ PSU Firmware
 * Copyright (C) 2017-present, Envox d.o.o.
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
#include "ntp.h"
#include "ethernet.h"
#include "datetime.h"
#include "persist_conf.h"
#if OPTION_WATCHDOG && (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12)
#include "watchdog.h"
#endif
#include <EthernetUdp2.h>

#if OPTION_ETHERNET

// Some time servers:
// - time.google.com
// - time.nist.gov


#define CONF_NTP_LOCAL_PORT 8888

#define CONF_PARSE_TIMEOUT_MS 5 * 1000 // 5 second
#define CONF_TIMEOUT_AFTER_SUCCESS_MS CONF_NTP_PERIOD_SEC * 1000L
#define CONF_TIMEOUT_AFTER_ERROR_MS CONF_NTP_PERIOD_AFTER_ERROR_SEC * 1000L

namespace eez {
namespace psu {
namespace ntp {

static EthernetUDP g_udp;

static const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
static byte packetBuffer[ NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets

static enum {
    STOPPED,
    START,
    PARSE,
    SUCCESS,
    ERROR
} g_state;

static uint32_t g_lastTickCount;

static const char *g_ntpServerToTest;

const char *getNtpServer() {
    if (g_ntpServerToTest) {
        if (g_ntpServerToTest[0]) {
            return g_ntpServerToTest;
        }
    } else {
        if (persist_conf::isNtpEnabled() && persist_conf::devConf2.ntpServer[0]) {
            return persist_conf::devConf2.ntpServer;
        }
    }
    return NULL;
}

// send an NTP request to the time server at the given address
int sendNtpPacket() {
      // set all bytes in the buffer to 0
      memset(packetBuffer, 0, NTP_PACKET_SIZE);

      // Initialize values needed to form NTP request
      // (see http://en.wikipedia.org/wiki/Network_Time_Protocol for details on the packets)
      packetBuffer[0] = 0b11100011;   // LI, Version, Mode
      packetBuffer[1] = 0;     // Stratum, or type of clock
      packetBuffer[2] = 6;     // Polling Interval
      packetBuffer[3] = 0xEC;  // Peer Clock Precision

      // 8 bytes of zero for Root Delay & Root Dispersion
      packetBuffer[12] = 49;
      packetBuffer[13] = 0x4E;
      packetBuffer[14] = 49;
      packetBuffer[15] = 52;

      // All NTP fields have been given values, now
      // you can send a packet requesting a timestamp:
      int rc;

#if OPTION_WATCHDOG && (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12)
      watchdog::disable();
#endif

      rc = g_udp.beginPacket(getNtpServer(), 123); // NTP requests are to port 123
      if (!rc) {
#if OPTION_WATCHDOG && (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12)
          watchdog::enable();
#endif
          return -1;
      }

      int written = g_udp.write(packetBuffer, NTP_PACKET_SIZE);
      if (written != NTP_PACKET_SIZE) {
#if OPTION_WATCHDOG && (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12)
          watchdog::enable();
#endif
          return -2;
      }

      rc = g_udp.endPacket();
      if (!rc) {
#if OPTION_WATCHDOG && (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12)
          watchdog::enable();
#endif
          return -3;
      }

#if OPTION_WATCHDOG && (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12)
      watchdog::enable();
#endif
      return 0;
}

void readNtpPacket() {
    // We've received a packet, read the data from it
#if OPTION_WATCHDOG && (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12)
    watchdog::disable();
#endif
    g_udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
#if OPTION_WATCHDOG && (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12)
    watchdog::enable();
#endif

    // The timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:
    uint32_t highWord = (packetBuffer[40] << 8) + packetBuffer[41];
    uint32_t lowWord = (packetBuffer[42] << 8) + packetBuffer[43];

    // Combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    uint32_t secsSince1900 = highWord << 16 | lowWord;

    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const uint32_t seventyYears = 2208988800UL;

    // Subtract seventy years:
    uint32_t utc = secsSince1900 - seventyYears;

    uint32_t local = datetime::utcToLocal(utc, persist_conf::devConf.time_zone, (datetime::DstRule)persist_conf::devConf2.dstRule);

    int year, month, day, hour, minute, second;
    datetime::breakTime(local, year, month, day, hour, minute, second);
   
    //DebugTraceF("NTP: %d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);

    datetime::setDateTime(year - 2000, month, day, hour, minute, second, false);
}

void begin() {
    g_udp.begin(CONF_NTP_LOCAL_PORT);
    g_state = START;
}

void init() {
    if (ethernet::g_testResult == TEST_OK && persist_conf::isNtpEnabled() && persist_conf::devConf2.ntpServer[0]) {
        begin();
    }
}

void tick(uint32_t tickCount) {
    if (ethernet::g_testResult == TEST_OK) {
        if (g_state == STOPPED) {
            begin();
        }

        tickCount = millis();

        if (g_state == START) {
            if (getNtpServer()) {
                //DebugTraceF("sendNtpPacket start %ul", micros());
                int rc = sendNtpPacket();
                if (rc >= 0) {
                    //DebugTraceF("sendNtpPacket success %ul", micros());
                    g_state = PARSE;
                    g_lastTickCount = tickCount;
                } else {
                    //DebugTraceF("sendNtpPacket fail %d %ul", rc, micros());
                    g_state = ERROR;
                    g_lastTickCount = tickCount;
                }
            } else {
                g_state = ERROR;
                g_lastTickCount = tickCount;
            }
        } else if (g_state == PARSE) {
            if (g_udp.parsePacket()) {
                readNtpPacket();
                g_state = SUCCESS;
                g_lastTickCount = tickCount;
            } else {
                if (tickCount - g_lastTickCount > CONF_PARSE_TIMEOUT_MS) {
                    g_state = ERROR;
                    g_lastTickCount = tickCount;
                }
            }
        } else if (g_state == SUCCESS) {
            if (tickCount - g_lastTickCount > CONF_TIMEOUT_AFTER_SUCCESS_MS) {
                g_state = START;
            }
        } else if (g_state == ERROR) {
            if (tickCount - g_lastTickCount > CONF_TIMEOUT_AFTER_ERROR_MS) {
                g_state = START;
            }
        }
    } else {
        if (g_state != STOPPED) {
            g_udp.stop();
            g_state = STOPPED;
        }
    }
}

void reset() {
    if (g_state == SUCCESS || g_state == ERROR) {
        g_state = START;
    }
}

void testNtpServer(const char *ntpServer) {
    g_ntpServerToTest = ntpServer;
    g_state = START;
}

bool isTestNtpServerDone(bool &result) {
    if (g_state == SUCCESS || g_state == ERROR || g_state == STOPPED) {
        g_ntpServerToTest = NULL;
        result = g_state == SUCCESS;
        return true;
    }
    return false;
}

}
}
} // namespace eez::psu::ntp

#endif