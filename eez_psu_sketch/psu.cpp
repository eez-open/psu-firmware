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
#include "serial_psu.h"
#include "ethernet.h"
#include "bp.h"
#include "adc.h"
#include "dac.h"
#include "ioexp.h"
#include "temperature.h"
#include "persist_conf.h"
#include "sound.h"
#include "board.h"
#include "rtc.h"
#include "datetime.h"
#include "eeprom.h"
#include "calibration.h"
#include "profile.h"
#if OPTION_DISPLAY
#include "gui.h"
#endif
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
#include "watchdog.h"
#include "fan.h"
#endif

#ifdef EEZ_PSU_SIMULATOR
#include "front_panel/control.h"
#endif

namespace eez {
namespace psu {

using namespace scpi;

static bool g_is_booted = false;
static bool g_power_is_up = false;
static bool g_test_power_up_delay = false;
static unsigned long g_power_down_time;
static bool g_is_time_critical_mode = false;

static float g_current_max_limit = NAN;

ontime::Counter g_powerOnTimeCounter(ontime::ON_TIME_COUNTER_POWER);

////////////////////////////////////////////////////////////////////////////////

static bool psu_reset(bool power_on);

////////////////////////////////////////////////////////////////////////////////

void boot() {
    bool success = true;

    // initialize shield
    eez_psu_init();

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
	watchdog::init();
	fan::test_start();
#endif

    bp::init();
    serial::init();
    success &= eeprom::init();

	g_powerOnTimeCounter.init();

    persist_conf::loadDevice(); // loads global configuration parameters

#if OPTION_DISPLAY
    gui::init();
#endif

    success &= rtc::init();
    success &= datetime::init();
    success &= ethernet::init();
	success &= temperature::init();

    // load channels calibration parameters
    for (int i = 0; i < CH_NUM; ++i) {
        persist_conf::loadChannelCalibration(&Channel::get(i));
    }

    // auto recall profile or ...
    profile::Parameters profile;
    if (persist_conf::isProfileAutoRecallEnabled() && profile::load(persist_conf::getProfileAutoRecallLocation(), &profile)) {
        success &= profile::recallFromProfile(&profile);
    }
    else {
        // ... reset
        success &= psu_reset(true);
    }

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
	success &= fan::init();
#endif

    // play beep if there is an error during boot procedure
    if (!success) {
        sound::playBeep();
    }

    /*
    using namespace persist_conf;

    DebugTraceF("%d", sizeof(BlockHeader));                                         // 6
    DebugTraceF("%d", offsetof(BlockHeader, checksum));                             // 0
    DebugTraceF("%d", offsetof(BlockHeader, version));                              // 4
    DebugTraceF("%d", sizeof(DeviceFlags));                                         // 2
    DebugTraceF("%d", sizeof(DeviceConfiguration));                                 // 32
    DebugTraceF("%d", offsetof(DeviceConfiguration, header));                       // 0
    DebugTraceF("%d", offsetof(DeviceConfiguration, calibration_password));         // 6
    DebugTraceF("%d", offsetof(DeviceConfiguration, flags));                        // 23
    DebugTraceF("%d", offsetof(DeviceConfiguration, date_year));                    // 25
    DebugTraceF("%d", offsetof(DeviceConfiguration, date_month));                   // 26
    DebugTraceF("%d", offsetof(DeviceConfiguration, date_day));                     // 27
    DebugTraceF("%d", offsetof(DeviceConfiguration, time_hour));                    // 28
    DebugTraceF("%d", offsetof(DeviceConfiguration, time_minute));                  // 29
    DebugTraceF("%d", offsetof(DeviceConfiguration, time_second));                  // 30
    DebugTraceF("%d", offsetof(DeviceConfiguration, profile_auto_recall_location)); // 31

    typedef Channel::CalibrationConfiguration CalibrationConfiguration;
    typedef Channel::CalibrationValueConfiguration CalibrationValueConfiguration;
    typedef Channel::CalibrationValuePointConfiguration CalibrationValuePointConfiguration;

    DebugTraceF("%d", sizeof(CalibrationConfiguration));
    DebugTraceF("%d", offsetof(CalibrationConfiguration, flags));
    DebugTraceF("%d", offsetof(CalibrationConfiguration, u));
    DebugTraceF("%d", offsetof(CalibrationConfiguration, i));
    DebugTraceF("%d", offsetof(CalibrationConfiguration, calibration_date));
    DebugTraceF("%d", offsetof(CalibrationConfiguration, calibration_remark));

    DebugTraceF("%d", sizeof(CalibrationValueConfiguration));
    DebugTraceF("%d", offsetof(CalibrationValueConfiguration, min));
    DebugTraceF("%d", offsetof(CalibrationValueConfiguration, max));
    DebugTraceF("%d", offsetof(CalibrationValueConfiguration, mid));

    DebugTraceF("%d", sizeof(CalibrationValuePointConfiguration));
    DebugTraceF("%d", offsetof(CalibrationValuePointConfiguration, dac));
    DebugTraceF("%d", offsetof(CalibrationValuePointConfiguration, val));
    DebugTraceF("%d", offsetof(CalibrationValuePointConfiguration, adc));

    using namespace profile;

    DebugTraceF("%d", sizeof(Parameters));
    DebugTraceF("%d", offsetof(Parameters, header));
    DebugTraceF("%d", offsetof(Parameters, is_valid));
    DebugTraceF("%d", offsetof(Parameters, name));
    DebugTraceF("%d", offsetof(Parameters, power_is_up));
    DebugTraceF("%d", offsetof(Parameters, channels));
    DebugTraceF("%d", offsetof(Parameters, temp_prot));

    DebugTraceF("%d", sizeof(ChannelParameters));
    DebugTraceF("%d", offsetof(ChannelParameters, flags));
    DebugTraceF("%d", offsetof(ChannelParameters, u_set));
    DebugTraceF("%d", offsetof(ChannelParameters, u_step));
    DebugTraceF("%d", offsetof(ChannelParameters, i_set));
    DebugTraceF("%d", offsetof(ChannelParameters, i_step));
    DebugTraceF("%d", offsetof(ChannelParameters, u_delay));
    DebugTraceF("%d", offsetof(ChannelParameters, u_level));
    DebugTraceF("%d", offsetof(ChannelParameters, i_delay));
    DebugTraceF("%d", offsetof(ChannelParameters, p_delay));
    DebugTraceF("%d", offsetof(ChannelParameters, p_level));

    using namespace temperature;

    DebugTraceF("%d", sizeof(ProtectionConfiguration));
    DebugTraceF("%d", offsetof(ProtectionConfiguration, sensor));
    DebugTraceF("%d", offsetof(ProtectionConfiguration, delay));
    DebugTraceF("%d", offsetof(ProtectionConfiguration, level));
    DebugTraceF("%d", offsetof(ProtectionConfiguration, state));
    */

    g_is_booted = true;
}

bool powerUp() {
    if (g_power_is_up) return true;

	if (!temperature::isAllowedToPowerUp()) return false;

#if OPTION_DISPLAY
    gui::showWelcomePage();
#endif

    // reset channels
    for (int i = 0; i < CH_NUM; ++i) {
        Channel::get(i).reset();
    }

    // turn power on
    board::powerUp();
    g_power_is_up = true;

    // turn off standby blue LED
    bp::switchStandby(false);

    bool success = true;

    // init channels
    for (int i = 0; i < CH_NUM; ++i) {
        success &= Channel::get(i).init();
    }

    // turn on Power On (PON) bit of ESE register
    setEsrBits(ESR_PON);

    // play power up tune on success
    if (success) {
		g_powerOnTimeCounter.start();
        sound::playPowerUp();
    }

    return success;
}

void powerDown() {
#if OPTION_DISPLAY
    if (g_is_booted) {
        gui::showEnteringStandbyPage();
    } else {
        gui::showStandbyPage();
    }
#endif

    if (!g_power_is_up) return;

    for (int i = 0; i < CH_NUM; ++i) {
        Channel::get(i).onPowerDown();
    }

    board::powerDown();

    // turn on standby blue LED
    bp::switchStandby(true);

    g_power_is_up = false;

	g_powerOnTimeCounter.stop();
    sound::playPowerDown();
}

bool isPowerUp() {
    return g_power_is_up;
}

bool changePowerState(bool up) {
    if (up == g_power_is_up) return true;

    if (up) {
        // at least MIN_POWER_UP_DELAY seconds shall pass after last power down
        if (g_test_power_up_delay) {
            if (millis() - g_power_down_time < MIN_POWER_UP_DELAY * 1000) return false;
            g_test_power_up_delay = false;
        }

        if (!powerUp()) {
            return false;
        }

        // auto recall channels parameters from profile
        profile::Parameters profile;
        if (persist_conf::isProfileAutoRecallEnabled() && profile::load(persist_conf::getProfileAutoRecallLocation(), &profile)) {
            profile::recallChannelsFromProfile(&profile);
        }
    
        profile::save();
    }
    else {
        g_power_is_up = false;
        profile::saveImmediately();
        g_power_is_up = true;

        profile::enableSave(false);

        powerDown();

        profile::enableSave(true);

        g_test_power_up_delay = true;
        g_power_down_time = millis();
    }

    return true;
}

void powerDownBySensor() {
    powerDown();
    profile::save();
}

static bool psu_reset(bool power_on) {
    if (!power_on) {
        powerDown();
    }

    if (power_on) {
        // *ESE 0
        // *SRE 0
        // *STB? 0
        // *ESR? 0

        // on power on, this is already set to 0 because in C
        // global and static variables are guaranteed to be initialized to 0
    }

    if (power_on) {
        // STAT:OPER[:EVEN] 0
        // STAT: OPER : COND 0
        // STAT: OPER : ENAB 0
        // STAT: OPER : INST[:EVEN] 0
        // STAT: OPER : INST : COND 0
        // STAT: OPER : INST : ENAB 0
        // STAT: OPER : INST : ISUM[:EVEN] 0
        // STAT: OPER : INST : ISUM : COND 0
        // STAT: OPER : INST : ISUM : ENAB 0
        // STAT: QUES[:EVEN] 0
        // STAT: QUES : COND 0
        // STAT: QUES : ENAB 0
        // STAT: QUES : INST[:EVEN] 0
        // STAT: QUES : INST : COND 0
        // STAT: QUES : INST : ENAB 0
        // STAT: QUES : INST : ISUM[:EVEN] 0
        // STAT: QUES : INST : ISUM : COND 0
        // STAT: OPER : INST : ISUM : ENAB 0

        // on power on, this is already set to 0 because in C
        // global and static variables are guaranteed to be initialized to 0
    }

    // SYST:ERR:COUN? 0
    SCPI_ErrorClear(&serial::scpi_context);
    if (ethernet::test_result == TEST_OK)
        SCPI_ErrorClear(&ethernet::scpi_context);

    // TEMP:PROT[MAIN]
    // TEMP:PROT:DEL
    // TEMP:PROT:STAT[MAIN]
    for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
        temperature::ProtectionConfiguration &temp_prot = temperature::sensors[i].prot_conf;
        temp_prot.sensor = i;
        temp_prot.delay = OTP_MAIN_DEFAULT_DELAY;
        temp_prot.level = OTP_MAIN_DEFAULT_LEVEL;
        temp_prot.state = OTP_MAIN_DEFAULT_STATE;
    }

    // CAL[:MODE] OFF
    calibration::stop();

    // SYST:POW ON
    if (powerUp()) {
        for (int i = 0; i < CH_NUM; ++i) {
            Channel::get(i).update();
        }
        return true;
    }

    return false;
}

bool reset() {
    if (psu_reset(false)) {
        profile::save();
        return true;
    }
    return false;
}

static bool test_channels() {
    if (!g_power_is_up) {
        // test is skipped
        return true;
    }

    bool result = true;

    for (int i = 0; i < CH_NUM; ++i) {
        result &= Channel::get(i).test();
    }

    return result;
}

static bool test_shield() {
    bool result = true;

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
	fan::test_start();
#endif

	result &= rtc::test();
    result &= datetime::test();
    result &= eeprom::test();
    result &= ethernet::test();
	result &= temperature::test();

	result &= test_channels();

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
	result &= fan::test();
#endif

    return result;
}

bool test() {
    bool result = true;
    result &= test_shield();
    result &= test_channels();
    if (!result) {
        sound::playBeep();
    }
    return result;
}

void tick() {
    unsigned long tick_usec = micros();

#if CONF_DEBUG
    debug::tick(tick_usec);
#endif

    temperature::tick(tick_usec);

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
	watchdog::tick(tick_usec);
	fan::tick(tick_usec);
#endif

    for (int i = 0; i < CH_NUM; ++i) {
        Channel::get(i).tick(tick_usec);
    }

    serial::tick(tick_usec);
    ethernet::tick(tick_usec);
    sound::tick(tick_usec);
    profile::tick(tick_usec);

	g_powerOnTimeCounter.tick(tick_usec);

#if OPTION_DISPLAY
    gui::tick(tick_usec);
#endif
}

void setEsrBits(int bit_mask) {
    SCPI_RegSetBits(&serial::scpi_context, SCPI_REG_ESR, bit_mask);
    if (ethernet::test_result == TEST_OK)
        SCPI_RegSetBits(&ethernet::scpi_context, SCPI_REG_ESR, bit_mask);
}


void setQuesBits(int bit_mask, bool on) {
    reg_set_ques_bit(&serial::scpi_context, bit_mask, on);
    if (ethernet::test_result == TEST_OK)
        reg_set_ques_bit(&ethernet::scpi_context, bit_mask, on);
}

void generateError(int16_t error) {
    SCPI_ErrorPush(&serial::scpi_context, error);
    if (ethernet::test_result == TEST_OK) {
        SCPI_ErrorPush(&ethernet::scpi_context, error);
    }
}

////////////////////////////////////////////////////////////////////////////////

#define MODEL_PREFIX "PSU"

#if defined(EEZ_PSU_SIMULATOR)
#define PLATFORM "Simulator"
#elif defined(EEZ_PSU_ARDUINO_MEGA)
#define PLATFORM "Mega"
#elif defined(EEZ_PSU_ARDUINO_DUE)
#define PLATFORM "Due"
#endif

#define MODEL_NUM_CHARS (sizeof(MODEL_PREFIX) - 1 + CH_NUM * sizeof("X/XX/XX") + 2 + sizeof(PLATFORM))

/*
We are auto generating model name from the channels definition:

<cnt>/<volt>/<curr>[-<cnt2>/<volt2>/<curr2>] (<platform>)

Where is:

<cnt>      - number of the equivalent channels
<volt>     - max. voltage
<curr>     - max. curr
<platform> - Mega, Due, Simulator or Unknown
*/
const char *getModelName() {
    static char model_name[MODEL_NUM_CHARS + 1];

    if (*model_name == 0) {
        strcat(model_name, MODEL_PREFIX);

        char *p = model_name + strlen(model_name);

        bool ch_used[CH_NUM];

        for (int i = 0; i < CH_NUM; ++i) {
            ch_used[i] = false;
        }

        bool first_channel = true;

        for (int i = 0; i < CH_NUM; ++i) {
            if (!ch_used[i]) {
                int count = 1;
                for (int j = i + 1; j < CH_NUM; ++j) {
                    if (Channel::get(i).U_MAX == Channel::get(j).U_MAX && Channel::get(i).I_MAX == Channel::get(j).I_MAX) {
                        ch_used[j] = true;
                        ++count;
                    }
                }

                if (first_channel) {
                    *p++ += ' ';
                    first_channel = false;
                }
                else {
                    *p++ += '-';
                }

                p += sprintf_P(p, PSTR("%d/%02d/%02d"), count, (int)floor(Channel::get(i).U_MAX), (int)floor(Channel::get(i).I_MAX));
            }
        }

        *p++ = ' ';
        *p++ = '(';
        strcpy(p, PLATFORM);
        p += sizeof(PLATFORM) - 1;
        *p++ += ')';
        *p = 0;
    }

    return model_name;
}

const char *getCpuModel() {
#if defined(EEZ_PSU_ARDUINO)
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    return "Arduino, R1B9";
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
    return "Arduino, R2B6";
#endif
#else
    return "Simulator, M2.0";
#endif
}

const char *getCpuType() {
    return PLATFORM;
}

const char *getCpuEthernetType() {
#if defined(EEZ_PSU_ARDUINO)
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    return "ENC28J60";
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
    return "W5500";
#endif
#else
    return "Simulator";
#endif
}

void enterTimeCriticalMode() {
    g_is_time_critical_mode = true;
}

bool isTimeCriticalMode() {
    return g_is_time_critical_mode;
}

void leaveTimeCriticalMode() {
    g_is_time_critical_mode = false;
}

extern void setCurrentMaxLimit(float value) {
	if (g_current_max_limit != value) {
		g_current_max_limit = value;

		if (g_current_max_limit != NAN) {
			for (int i = 0; i < CH_NUM; ++i) {
				if (g_current_max_limit < Channel::get(i).getCurrentLimit()) {
					Channel::get(i).setCurrentLimit(g_current_max_limit);
				}
			}
		}
	}
}

extern float getCurrentMaxLimit() {
	return g_current_max_limit;
}

}
} // namespace eez::psu


#if defined(EEZ_PSU_ARDUINO)

void PSU_boot() { 
    eez::psu::boot(); 
}

void PSU_tick() { 
    eez::psu::tick (); 
}

#endif 
