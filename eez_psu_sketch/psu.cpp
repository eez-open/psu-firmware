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

#if OPTION_ETHERNET
#include "ethernet.h"
#endif

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
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
#include "watchdog.h"
#include "fan.h"
#endif

#ifdef EEZ_PSU_SIMULATOR
#include "front_panel/control.h"
#endif

#include "event_queue.h"

namespace eez {
namespace psu {

using namespace scpi;

static bool g_is_booted = false;
static bool g_power_is_up = false;
static bool g_test_power_up_delay = false;
static unsigned long g_power_down_time;
static bool g_is_time_critical_mode = false;

static MaxCurrentLimitCause g_maxCurrentLimitCause;

ontime::Counter g_powerOnTimeCounter(ontime::ON_TIME_COUNTER_POWER);

bool g_insideInterruptHandler = false;

////////////////////////////////////////////////////////////////////////////////

static bool psu_reset(bool power_on);
static bool test_shield();

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 && OPTION_SYNC_MASTER && !defined(EEZ_PSU_SIMULATOR)
static void startMasterSync();
static void updateMasterSync();
#endif

////////////////////////////////////////////////////////////////////////////////

void boot() {
    bool success = true;

    // initialize shield
    eez_psu_init();

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 && OPTION_SYNC_MASTER && !defined(EEZ_PSU_SIMULATOR)
	startMasterSync();
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

	event_queue::init();

#if OPTION_ETHERNET
    gui::showEthernetInit();
	success &= ethernet::init();
#else
	DebugTrace("Ethernet initialization skipped!");
#endif

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
	fan::test_start();
#endif

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

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
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

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
	watchdog::init();
#endif
}

bool powerUp() {
    if (g_power_is_up) return true;

	if (!temperature::isAllowedToPowerUp()) return false;

#if OPTION_DISPLAY
    gui::showWelcomePage();
#endif

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
	if (g_is_booted) {
		fan::test_start();
	}
#endif

    // reset channels
    for (int i = 0; i < CH_NUM; ++i) {
        Channel::get(i).reset();
    }

    // turn power on
    board::powerUp();
    g_power_is_up = true;
	g_powerOnTimeCounter.start();

    // turn off standby blue LED
    bp::switchStandby(false);

    bool success = true;

	if (g_is_booted) {
		success &= test_shield();
	}

    // init channels
    for (int i = 0; i < CH_NUM; ++i) {
        success &= Channel::get(i).init();
    }

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
	if (g_is_booted) {
		success &= fan::test();
	}
#endif

    // turn on Power On (PON) bit of ESE register
    setEsrBits(ESR_PON);

	event_queue::pushEvent(event_queue::EVENT_INFO_POWER_UP);

    // play power up tune on success
    if (success) {
        sound::playPowerUp();
    }

    return true;
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

	event_queue::pushEvent(event_queue::EVENT_INFO_POWER_DOWN);

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
#if OPTION_ETHERNET
	if (ethernet::test_result == TEST_OK) {
        SCPI_ErrorClear(&ethernet::scpi_context);
	}
#endif

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

	result &= rtc::test();
    result &= datetime::test();
    result &= eeprom::test();

#if OPTION_ETHERNET
    result &= ethernet::test();
#endif

	result &= temperature::test();

    return result;
}

bool test() {
    bool result = true;

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
	fan::test_start();
#endif

    result &= test_shield();
    result &= test_channels();

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
	result &= fan::test();
#endif

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

	g_powerOnTimeCounter.tick(tick_usec);

	temperature::tick(tick_usec);

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
	watchdog::tick(tick_usec);
	fan::tick(tick_usec);
#endif

    for (int i = 0; i < CH_NUM; ++i) {
        Channel::get(i).tick(tick_usec);
    }

    serial::tick(tick_usec);

#if OPTION_ETHERNET
	ethernet::tick(tick_usec);
#endif
    
	sound::tick(tick_usec);
    profile::tick(tick_usec);

#if OPTION_DISPLAY
    gui::tick(tick_usec);
#endif

	event_queue::tick(tick_usec);

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 && OPTION_SYNC_MASTER && !defined(EEZ_PSU_SIMULATOR)
	updateMasterSync();
#endif
}

void setEsrBits(int bit_mask) {
    SCPI_RegSetBits(&serial::scpi_context, SCPI_REG_ESR, bit_mask);
#if OPTION_ETHERNET
	if (ethernet::test_result == TEST_OK) {
        SCPI_RegSetBits(&ethernet::scpi_context, SCPI_REG_ESR, bit_mask);
	}
#endif
}


void setQuesBits(int bit_mask, bool on) {
    reg_set_ques_bit(&serial::scpi_context, bit_mask, on);
#if OPTION_ETHERNET
	if (ethernet::test_result == TEST_OK) {
        reg_set_ques_bit(&ethernet::scpi_context, bit_mask, on);
	}
#endif
}

void generateError(int16_t error) {
    SCPI_ErrorPush(&serial::scpi_context, error);
#if OPTION_ETHERNET
	if (ethernet::test_result == TEST_OK) {
        SCPI_ErrorPush(&ethernet::scpi_context, error);
    }
#endif
	event_queue::pushEvent(error);
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

		p = Channel::getChannelsInfo(p);

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
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
    return "Arduino, R3B4";
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
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
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

bool isMaxCurrentLimited() {
    return g_maxCurrentLimitCause != MAX_CURRENT_LIMIT_CAUSE_NONE;
}

void limitMaxCurrent(MaxCurrentLimitCause cause) {
	if (g_maxCurrentLimitCause != cause) {
		g_maxCurrentLimitCause = cause;

		if (isMaxCurrentLimited()) {
			for (int i = 0; i < CH_NUM; ++i) {
				if (Channel::get(i).isOutputEnabled() && Channel::get(i).i.mon > ERR_MAX_CURRENT) {
					Channel::get(i).setCurrent(Channel::get(i).i.min);
				}

				if (ERR_MAX_CURRENT < Channel::get(i).getCurrentLimit()) {
					Channel::get(i).setCurrentLimit(ERR_MAX_CURRENT);
				}
			}
		}
	}
}

void unlimitMaxCurrent() {
    limitMaxCurrent(MAX_CURRENT_LIMIT_CAUSE_NONE);
}

MaxCurrentLimitCause getMaxCurrentLimitCause() {
    return g_maxCurrentLimitCause;
}

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 && OPTION_SYNC_MASTER && !defined(EEZ_PSU_SIMULATOR)
static bool g_masterSyncStarted;
static Tc *g_chTC;
static uint32_t g_chNo;

static void TC_SetCMR_ChannelA(Tc *tc, uint32_t chan, uint32_t v) {
	tc->TC_CHANNEL[chan].TC_CMR = (tc->TC_CHANNEL[chan].TC_CMR & 0xFFF0FFFF) | v;
}

static void TC_SetCMR_ChannelB(Tc *tc, uint32_t chan, uint32_t v) {
	tc->TC_CHANNEL[chan].TC_CMR = (tc->TC_CHANNEL[chan].TC_CMR & 0xF0FFFFFF) | v;
}

/// Generate square wave of frequency 330kHz on SYNC_MASTER pin
void startMasterSync() {
    // We use MCLK/2 as clock.
    const uint32_t frequency = SYNC_MASTER_FREQUENCY;
    const uint32_t TC = VARIANT_MCK / 2 / frequency; // VARIANT_MCK = 84000000

    // Map value to Timer ranges 0..255 => 0..TC
    uint32_t ulValue = 127;
    ulValue = ulValue * TC;
    ulValue = ulValue / TC_MAX_DUTY_CYCLE;

    // Setup Timer for this pin
    ETCChannel channel = g_APinDescription[SYNC_MASTER].ulTCChannel;

    static const uint32_t channelToChNo[] = { 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2 };
    static const uint32_t channelToAB[]   = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
    static Tc *channelToTC[] = {
        TC0, TC0, TC0, TC0, TC0, TC0,
        TC1, TC1, TC1, TC1, TC1, TC1,
        TC2, TC2, TC2, TC2, TC2, TC2 };
    static const uint32_t channelToId[] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8 };
    uint32_t chNo = channelToChNo[channel];
    uint32_t chA  = channelToAB[channel];
    Tc *chTC = channelToTC[channel];
    uint32_t interfaceID = channelToId[channel];

    pmc_enable_periph_clk(TC_INTERFACE_ID + interfaceID);
    TC_Configure(chTC, chNo,
        TC_CMR_TCCLKS_TIMER_CLOCK1 |
        TC_CMR_WAVE |         // Waveform mode
        TC_CMR_WAVSEL_UP_RC | // Counter running up and reset when equals to RC
        TC_CMR_EEVT_XC0 |     // Set external events from XC0 (this setup TIOB as output)
        TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_CLEAR |
        TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_CLEAR);
    TC_SetRC(chTC, chNo, TC);

    if (chA) {
        TC_SetRA(chTC, chNo, ulValue);
        TC_SetCMR_ChannelA(chTC, chNo, TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);
    } else {
        TC_SetRB(chTC, chNo, ulValue);
        TC_SetCMR_ChannelB(chTC, chNo, TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_SET);
    }

    PIO_Configure(g_APinDescription[SYNC_MASTER].pPort,
            g_APinDescription[SYNC_MASTER].ulPinType,
            g_APinDescription[SYNC_MASTER].ulPin,
            g_APinDescription[SYNC_MASTER].ulPinConfiguration);

    TC_Start(chTC, chNo);
	g_chTC = chTC;
	g_chNo = chNo;
	g_masterSyncStarted = true;
}

void updateMasterSync() {
#if 0
	bool shouldBeStarted = true;

    for (int i = 0; i < CH_NUM; ++i) {
		if (Channel::get(i).isLowRippleEnabled()) {
			shouldBeStarted = false;
			break;
		}
    }

	if (shouldBeStarted != g_masterSyncStarted) {
		if (shouldBeStarted) {
			TC_Start(g_chTC, g_chNo);
		} else {
			TC_Stop(g_chTC, g_chNo);
		}
		g_masterSyncStarted = shouldBeStarted;
	}
#endif
}

#endif


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
