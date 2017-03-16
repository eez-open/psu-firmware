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
#if OPTION_SD_CARD
#include "sd_card.h"
#endif
#include "calibration.h"
#include "profile.h"
#if OPTION_DISPLAY
#include "gui.h"
#include "touch.h"
#endif
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
#if OPTION_WATCHDOG
#include "watchdog.h"
#endif
#include "fan.h"
#endif

#ifdef EEZ_PSU_SIMULATOR
#include "front_panel/control.h"
#endif

#include "event_queue.h"
#include "channel_dispatcher.h"
#include "trigger.h"
#include "list.h"

namespace eez {
namespace psu {

using namespace scpi;

bool g_isBooted = false;
static bool g_bootTestSuccess;
static bool g_powerIsUp = false;
static bool g_testPowerUpDelay = false;
static uint32_t g_powerDownTime;
static bool g_isTimeCriticalMode = false;

static MaxCurrentLimitCause g_maxCurrentLimitCause;

ontime::Counter g_powerOnTimeCounter(ontime::ON_TIME_COUNTER_POWER);

volatile bool g_insideInterruptHandler = false;
static bool g_shutdownOnNextTick;

RLState g_rlState = RL_STATE_LOCAL;

static uint32_t g_mainLoopCounter;

////////////////////////////////////////////////////////////////////////////////

static bool testShield();

#if (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12) && OPTION_SYNC_MASTER && !defined(EEZ_PSU_SIMULATOR)
static void startMasterSync();
static void updateMasterSync();
#endif

////////////////////////////////////////////////////////////////////////////////

void loadConf() {
    // loads global configuration parameters
    persist_conf::loadDevice();
 
    // loads global configuration parameters block 2
    persist_conf::loadDevice2();

    // load channels calibration parameters
    for (int i = 0; i < CH_NUM; ++i) {
        persist_conf::loadChannelCalibration(&Channel::get(i));
    }
}

////////////////////////////////////////////////////////////////////////////////

void init() {
    // initialize shield
    eez_psu_init();

    analogWrite(LCD_BRIGHTNESS, 255);

#if (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12) && OPTION_SYNC_MASTER && !defined(EEZ_PSU_SIMULATOR)
	startMasterSync();
#endif

    bp::init();
    serial::init();

    eeprom::init();
    eeprom::test();

#if OPTION_SD_CARD
    sd_card::init();
#endif

	g_powerOnTimeCounter.init();
    for (int i = 0; i < CH_NUM; ++i) {
        Channel::get(i).onTimeCounter.init();
    }

    loadConf();

#if OPTION_DISPLAY
    gui::init();
#endif

    rtc::init();
	datetime::init();

	event_queue::init();

    list::init();

#if OPTION_ETHERNET
#if OPTION_DISPLAY
    gui::showEthernetInit();
#endif
	ethernet::init();
#else
	DebugTrace("Ethernet initialization skipped!");
#endif

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
    fan::init();
#endif

	temperature::init();

    trigger::init();
}

////////////////////////////////////////////////////////////////////////////////

static bool testChannels() {
    if (!g_powerIsUp) {
        // test is skipped
        return true;
    }

    bool result = true;

    for (int i = 0; i < CH_NUM; ++i) {
        result &= Channel::get(i).test();
    }

    return result;
}

static bool testShield() {
    bool result = true;

	result &= rtc::test();
    result &= datetime::test();
    result &= eeprom::test();
#if OPTION_SD_CARD
    result &= sd_card::test();
#endif

#if OPTION_ETHERNET
    result &= ethernet::test();
#endif

	result &= temperature::test();

    return result;
}

bool test() {
    bool testResult = true;

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
	fan::test_start();
#endif

    testResult &= testShield();
    testResult &= testChannels();

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
	testResult &= fan::test();
#endif

	if (!testResult) {
        sound::playBeep();
    }

	return testResult;
}

////////////////////////////////////////////////////////////////////////////////

static bool psuReset() {
    // *ESE 0
    SCPI_RegSet(&serial::scpi_context, SCPI_REG_ESE, 0);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        SCPI_RegSet(&ethernet::scpi_context, SCPI_REG_ESE, 0);
	}
#endif

    // *SRE 0
    SCPI_RegSet(&serial::scpi_context, SCPI_REG_ESE, 0);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        SCPI_RegSet(&ethernet::scpi_context, SCPI_REG_SRE, 0);
	}
#endif

    // *STB 0
    SCPI_RegSet(&serial::scpi_context, SCPI_REG_ESE, 0);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        SCPI_RegSet(&ethernet::scpi_context, SCPI_REG_STB, 0);
	}
#endif

    // *ESR 0
    SCPI_RegSet(&serial::scpi_context, SCPI_REG_ESE, 0);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        SCPI_RegSet(&ethernet::scpi_context, SCPI_REG_ESR, 0);
	}
#endif

    // STAT:OPER[:EVEN] 0
    SCPI_RegSet(&serial::scpi_context, SCPI_REG_OPER, 0);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        SCPI_RegSet(&ethernet::scpi_context, SCPI_REG_OPER, 0);
	}
#endif

    // STAT:OPER:COND 0
    reg_set(&serial::scpi_context, SCPI_PSU_REG_OPER_COND, 0);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_REG_OPER_COND, 0);
	}
#endif

    // STAT:OPER:ENAB 0
    SCPI_RegSet(&serial::scpi_context, SCPI_REG_OPERE, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        SCPI_RegSet(&ethernet::scpi_context, SCPI_REG_OPERE, 0);
	}
#endif

    // STAT:OPER:INST[:EVEN] 0
    reg_set(&serial::scpi_context, SCPI_PSU_REG_OPER_INST_EVENT, 0);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_REG_OPER_INST_EVENT, 0);
	}
#endif

    // STAT:OPER:INST:COND 0
    reg_set(&serial::scpi_context, SCPI_PSU_REG_OPER_INST_COND, 0);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_REG_OPER_INST_COND, 0);
	}
#endif

    // STAT:OPER:INST:ENAB 0
    reg_set(&serial::scpi_context, SCPI_PSU_REG_OPER_INST_ENABLE, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_REG_OPER_INST_ENABLE, 0);
	}
#endif

    // STAT:OPER:INST:ISUM[:EVEN] 0
    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT1, 0);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT1, 0);
	}
#endif

    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT2, 0);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT2, 0);
	}
#endif

    // STAT:OPER:INST:ISUM:COND 0
    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_COND1, 0);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_COND1, 0);
	}
#endif

    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_COND2, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_COND2, 0);
	}
#endif

    // STAT:OPER:INST:ISUM:ENAB 0
    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE1, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE1, 0);
	}
#endif

    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE2, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE2, 0);
	}
#endif

    // STAT:QUES[:EVEN] 0
    SCPI_RegSet(&serial::scpi_context, SCPI_REG_QUES, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        SCPI_RegSet(&ethernet::scpi_context, SCPI_REG_QUES, 0);
	}
#endif

    // STAT:QUES:COND 0
    reg_set(&serial::scpi_context, SCPI_PSU_REG_QUES_COND, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_REG_QUES_COND, 0);
	}
#endif

    // STAT:QUES:ENAB 0
    SCPI_RegSet(&serial::scpi_context, SCPI_REG_QUESE, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        SCPI_RegSet(&ethernet::scpi_context, SCPI_REG_QUESE, 0);
	}
#endif

    // STAT:QUES:INST[:EVEN] 0
    reg_set(&serial::scpi_context, SCPI_PSU_REG_QUES_INST_EVENT, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_REG_QUES_INST_EVENT, 0);
	}
#endif

    // STAT:QUES:INST:COND 0
    reg_set(&serial::scpi_context, SCPI_PSU_REG_QUES_INST_COND, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_REG_QUES_INST_COND, 0);
	}
#endif

    // STAT:QUES:INST:ENAB 0
    reg_set(&serial::scpi_context, SCPI_PSU_REG_QUES_INST_ENABLE, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_REG_QUES_INST_ENABLE, 0);
	}
#endif

    // STAT:QUES:INST:ISUM[:EVEN] 0
    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT1, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT1, 0);
	}
#endif

    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT2, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT2, 0);
	}
#endif

    // STAT:QUES:INST:ISUM:COND 0
    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_COND1, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_COND1, 0);
	}
#endif

    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_COND2, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_COND2, 0);
	}
#endif

    // STAT:OPER:INST:ISUM:ENAB 0
    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE1, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE1, 0);
	}
#endif

    reg_set(&serial::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE2, 0);
#if OPTION_ETHERNET
    if (ethernet::g_testResult == TEST_OK) {
        reg_set(&ethernet::scpi_context, SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE2, 0);
	}
#endif

    // SYST:ERR:COUN? 0
    SCPI_ErrorClear(&serial::scpi_context);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        SCPI_ErrorClear(&ethernet::scpi_context);
	}
#endif

    // TEMP:PROT[AUX]
    // TEMP:PROT:DEL
    // TEMP:PROT:STAT[AUX]
    for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
        temperature::ProtectionConfiguration &temp_prot = temperature::sensors[i].prot_conf;
        temp_prot.sensor = i;
        if (temp_prot.sensor == temp_sensor::AUX) {
            temp_prot.delay = OTP_AUX_DEFAULT_DELAY;
            temp_prot.level = OTP_AUX_DEFAULT_LEVEL;
            temp_prot.state = OTP_AUX_DEFAULT_STATE;
        } else {
            temp_prot.delay = OTP_CH_DEFAULT_DELAY;
            temp_prot.level = OTP_CH_DEFAULT_LEVEL;
            temp_prot.state = OTP_CH_DEFAULT_STATE;
        }
    }

    // CAL[:MODE] OFF
    calibration::stop();

    // reset channels
    channel_dispatcher::setType(channel_dispatcher::TYPE_NONE);
    for (int i = 0; i < CH_NUM; ++i) {
        Channel::get(i).reset();
    }

    //
    trigger::reset();

    //
    list::reset();

    // SYST:POW ON
    if (powerUp()) {
        for (int i = 0; i < CH_NUM; ++i) {
            Channel::get(i).update();
        }
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////

static bool loadAutoRecallProfile(profile::Parameters *profile, int *location) {
    if (persist_conf::isProfileAutoRecallEnabled()) {
        *location = persist_conf::getProfileAutoRecallLocation();
        if (profile::load(*location, profile)) {
            bool outputEnabled = false;

            for (int i = 0; i < CH_NUM; ++i) {
                if (profile->channels[i].flags.output_enabled) {
                    outputEnabled = true;
                    break;
                }
            }

            if (outputEnabled) {
                bool disableOutputs = false;

                if (persist_conf::isForceDisablingAllOutputsOnPowerUpEnabled() || !g_bootTestSuccess) {
                    disableOutputs = true;
                } else {
                    if (*location != 0) {
                        profile::Parameters defaultProfile;
                        if (profile::load(0, &defaultProfile)) {
                            if (profile->flags.channelsCoupling != defaultProfile.flags.channelsCoupling) {
                                disableOutputs = true;
                                event_queue::pushEvent(event_queue::EVENT_WARNING_AUTO_RECALL_VALUES_MISMATCH);
                            } else {
                                for (int i = 0; i < CH_NUM; ++i) {
                                    if (!util::equal(profile->channels[i].u_set, defaultProfile.channels[i].u_set, getPrecision(VALUE_TYPE_FLOAT_VOLT)) ||
                                        !util::equal(profile->channels[i].i_set, defaultProfile.channels[i].i_set, getPrecision(VALUE_TYPE_FLOAT_AMPER))) 
                                    {
                                        disableOutputs = true;
                                        event_queue::pushEvent(event_queue::EVENT_WARNING_AUTO_RECALL_VALUES_MISMATCH);
                                        break;
                                    }
                                }
                            }
                        } else {
                            disableOutputs = true;
                        }
                    }
                }

                if (disableOutputs) {
                    for (int i = 0; i < CH_NUM; ++i) {
                        profile->channels[i].flags.output_enabled = false;
                    }
                }
            }

            return true;
        }
    }

    return false;
}

static bool autoRecall() {
    profile::Parameters profile;
    int location;
    if (loadAutoRecallProfile(&profile, &location)) {
        if (profile::recallFromProfile(&profile, location)) {
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void boot() {
    init();

    // test
    g_bootTestSuccess = true;

    g_bootTestSuccess &= testShield();

    if (!autoRecall()) {
        psuReset();
    }

    // play beep if there is an error during boot procedure
    if (!g_bootTestSuccess) {
        sound::playBeep();
    }

    g_isBooted = true;

    if (g_powerIsUp) {
        // enable channels output if required
        for (int i = 0; i < CH_NUM; ++i) {
            Channel::get(i).afterBootOutputEnable();
        }
    }

//    using namespace persist_conf;
//
//    DebugTraceF("%d", sizeof(BlockHeader));                                         // 8
//    DebugTraceF("%d", offsetof(BlockHeader, checksum));                             // 0
//    DebugTraceF("%d", offsetof(BlockHeader, version));                              // 4
//
//    DebugTraceF("%d", sizeof(DeviceFlags));                                         // 4
//
//    DebugTraceF("%d", sizeof(DeviceConfiguration));                                 // 64
//    DebugTraceF("%d", offsetof(DeviceConfiguration, header));                       // 0
//    DebugTraceF("%d", offsetof(DeviceConfiguration, serialNumber));                 // 8
//    DebugTraceF("%d", offsetof(DeviceConfiguration, calibration_password));         // 16
//    DebugTraceF("%d", offsetof(DeviceConfiguration, flags));                        // 36
//    DebugTraceF("%d", offsetof(DeviceConfiguration, date_year));                    // 40
//    DebugTraceF("%d", offsetof(DeviceConfiguration, date_month));                   // 41
//    DebugTraceF("%d", offsetof(DeviceConfiguration, date_day));                     // 42
//    DebugTraceF("%d", offsetof(DeviceConfiguration, time_hour));                    // 43
//    DebugTraceF("%d", offsetof(DeviceConfiguration, time_minute));                  // 44
//    DebugTraceF("%d", offsetof(DeviceConfiguration, time_second));                  // 45
//    DebugTraceF("%d", offsetof(DeviceConfiguration, time_zone));                    // 46
//    DebugTraceF("%d", offsetof(DeviceConfiguration, profile_auto_recall_location)); // 48
//    DebugTraceF("%d", offsetof(DeviceConfiguration, touch_screen_cal_orientation)); // 49
//    DebugTraceF("%d", offsetof(DeviceConfiguration, touch_screen_cal_tlx));         // 50
//    DebugTraceF("%d", offsetof(DeviceConfiguration, touch_screen_cal_tly));         // 52
//    DebugTraceF("%d", offsetof(DeviceConfiguration, touch_screen_cal_brx));         // 54
//    DebugTraceF("%d", offsetof(DeviceConfiguration, touch_screen_cal_bry));         // 56
//    DebugTraceF("%d", offsetof(DeviceConfiguration, touch_screen_cal_trx));         // 58
//    DebugTraceF("%d", offsetof(DeviceConfiguration, touch_screen_cal_try));         // 60
//#ifdef EEZ_PSU_SIMULATOR
//    DebugTraceF("%d", offsetof(DeviceConfiguration, gui_opened));                   // 62
//#endif // EEZ_PSU_SIMULATOR
//
//    DebugTraceF("%d", sizeof(DeviceConfiguration2));                                // 128
//
//    typedef Channel::CalibrationConfiguration CalibrationConfiguration;
//    typedef Channel::CalibrationValueConfiguration CalibrationValueConfiguration;
//    typedef Channel::CalibrationValuePointConfiguration CalibrationValuePointConfiguration;
//
//    DebugTraceF("%d", sizeof(CalibrationConfiguration));                            // 144
//    DebugTraceF("%d", offsetof(CalibrationConfiguration, header));                  // 0
//    DebugTraceF("%d", offsetof(CalibrationConfiguration, flags));                   // 8
//    DebugTraceF("%d", offsetof(CalibrationConfiguration, u));                       // 12
//    DebugTraceF("%d", offsetof(CalibrationConfiguration, i));                       // 56
//    DebugTraceF("%d", offsetof(CalibrationConfiguration, calibration_date));        // 100
//    DebugTraceF("%d", offsetof(CalibrationConfiguration, calibration_remark));      // 109
//
//    DebugTraceF("%d", sizeof(CalibrationValueConfiguration));                       // 44
//    DebugTraceF("%d", offsetof(CalibrationValueConfiguration, min));                // 0
//    DebugTraceF("%d", offsetof(CalibrationValueConfiguration, mid));                // 12
//    DebugTraceF("%d", offsetof(CalibrationValueConfiguration, max));                // 24
//    DebugTraceF("%d", offsetof(CalibrationValueConfiguration, minPossible));        // 36
//    DebugTraceF("%d", offsetof(CalibrationValueConfiguration, maxPossible));        // 40
//
//    DebugTraceF("%d", sizeof(CalibrationValuePointConfiguration));                  // 12
//    DebugTraceF("%d", offsetof(CalibrationValuePointConfiguration, dac));           // 0
//    DebugTraceF("%d", offsetof(CalibrationValuePointConfiguration, val));           // 4
//    DebugTraceF("%d", offsetof(CalibrationValuePointConfiguration, adc));           // 8
//
//    using namespace profile;
//
//    DebugTraceF("%d", sizeof(Parameters));                                          // 256
//    DebugTraceF("%d", offsetof(Parameters, header));                                // 0
//    DebugTraceF("%d", offsetof(Parameters, flags));                                 // 8
//    DebugTraceF("%d", offsetof(Parameters, name));                                  // 12
//    DebugTraceF("%d", offsetof(Parameters, channels));                              // 48
//    DebugTraceF("%d", offsetof(Parameters, temp_prot));                             // 176
//
//    DebugTraceF("%d", sizeof(ChannelParameters));                                   // 64
//    DebugTraceF("%d", offsetof(ChannelParameters, flags));                          // 0
//    DebugTraceF("%d", offsetof(ChannelParameters, u_set));                          // 4
//    DebugTraceF("%d", offsetof(ChannelParameters, u_step));                         // 8
//    DebugTraceF("%d", offsetof(ChannelParameters, u_limit));                        // 12
//    DebugTraceF("%d", offsetof(ChannelParameters, u_delay));                        // 16
//    DebugTraceF("%d", offsetof(ChannelParameters, u_level));                        // 20
//    DebugTraceF("%d", offsetof(ChannelParameters, i_set));                          // 24
//    DebugTraceF("%d", offsetof(ChannelParameters, i_step));                         // 28
//    DebugTraceF("%d", offsetof(ChannelParameters, i_limit));                        // 32
//    DebugTraceF("%d", offsetof(ChannelParameters, i_delay));                        // 36
//    DebugTraceF("%d", offsetof(ChannelParameters, p_limit));                        // 40
//    DebugTraceF("%d", offsetof(ChannelParameters, p_delay));                        // 44
//    DebugTraceF("%d", offsetof(ChannelParameters, p_level));                        // 48
//#ifdef EEZ_PSU_SIMULATOR
//    DebugTraceF("%d", offsetof(ChannelParameters, load_enabled));                   // 52
//    DebugTraceF("%d", offsetof(ChannelParameters, load));                           // 56
//    DebugTraceF("%d", offsetof(ChannelParameters, voltProgExt));                    // 60
//#endif // EEZ_PSU_SIMULATOR
//
//    using namespace temperature;
//
//    DebugTraceF("%d", sizeof(ProtectionConfiguration));                             // 16
//    DebugTraceF("%d", offsetof(ProtectionConfiguration, sensor));                   // 0
//    DebugTraceF("%d", offsetof(ProtectionConfiguration, delay));                    // 4
//    DebugTraceF("%d", offsetof(ProtectionConfiguration, level));                    // 8
//    DebugTraceF("%d", offsetof(ProtectionConfiguration, state));                    // 12
//
//    using namespace event_queue;
//
//    DebugTraceF("%d", sizeof(EventQueueHeader));                                    // 12
//    DebugTraceF("%d", offsetof(EventQueueHeader, magicNumber));                     // 0
//    DebugTraceF("%d", offsetof(EventQueueHeader, version));                         // 4
//    DebugTraceF("%d", offsetof(EventQueueHeader, head));                            // 6
//    DebugTraceF("%d", offsetof(EventQueueHeader, size));                            // 8
//    DebugTraceF("%d", offsetof(EventQueueHeader, lastErrorEventIndex));             // 10
//
//    DebugTraceF("%d", sizeof(Event));                                               // 8
//    DebugTraceF("%d", offsetof(Event, dateTime));                                   // 0
//    DebugTraceF("%d", offsetof(Event, eventId));                                    // 4
}

////////////////////////////////////////////////////////////////////////////////

bool powerUp() {
    if (g_powerIsUp) {
        return true;
    }

	if (!temperature::isAllowedToPowerUp()) {
        return false;
    }

    g_rlState = persist_conf::devConf.flags.isFrontPanelLocked ? RL_STATE_REMOTE : RL_STATE_LOCAL;

#if OPTION_DISPLAY
    gui::showWelcomePage();
#endif

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
    fan::test_start();
#endif

    // reset channels
    for (int i = 0; i < CH_NUM; ++i) {
        Channel::get(i).reset();
    }

    // turn power on
    board::powerUp();
    g_powerIsUp = true;
	g_powerOnTimeCounter.start();

    // turn off standby blue LED
    bp::switchStandby(false);

    // init channels
    for (int i = 0; i < CH_NUM; ++i) {
        Channel::get(i).init();
    }

    bool testSuccess = true;

    if (g_isBooted) {
	    testSuccess &= testShield();
    }

    // test channels
    for (int i = 0; i < CH_NUM; ++i) {
        testSuccess &= Channel::get(i).test();
    }

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
	testSuccess &= fan::test();
#endif

    // turn on Power On (PON) bit of ESE register
    setEsrBits(ESR_PON);

	event_queue::pushEvent(event_queue::EVENT_INFO_POWER_UP);

    // play power up tune on success
    if (testSuccess) {
        sound::playPowerUp();
    }

    g_bootTestSuccess &= testSuccess;

    return true;
}

void powerDown() {
#if OPTION_DISPLAY
    if (g_isBooted) {
        gui::showEnteringStandbyPage();
    } else {
        gui::showStandbyPage();
    }
#endif

    if (!g_powerIsUp) return;

    trigger::abort();

    channel_dispatcher::setType(channel_dispatcher::TYPE_NONE);

    for (int i = 0; i < CH_NUM; ++i) {
        Channel::get(i).onPowerDown();
    }

    board::powerDown();

    // turn on standby blue LED
    bp::switchStandby(true);

    g_powerIsUp = false;
	g_powerOnTimeCounter.stop();

	event_queue::pushEvent(event_queue::EVENT_INFO_POWER_DOWN);

	sound::playPowerDown();
}

bool isPowerUp() {
    return g_powerIsUp;
}

bool changePowerState(bool up) {
    if (up == g_powerIsUp) return true;

    if (up) {
        // at least MIN_POWER_UP_DELAY seconds shall pass after last power down
        if (g_testPowerUpDelay) {
            if (millis() - g_powerDownTime < MIN_POWER_UP_DELAY * 1000) return false;
            g_testPowerUpDelay = false;
        }

        g_bootTestSuccess = true;

        if (!powerUp()) {
            return false;
        }

        // auto recall channels parameters from profile
        profile::Parameters profile;
        int location;
        if (loadAutoRecallProfile(&profile, &location)) {
	        for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
		        memcpy(&temperature::sensors[i].prot_conf, profile.temp_prot + i, sizeof(temperature::ProtectionConfiguration));
	        }
            profile::recallChannelsFromProfile(&profile, location);
        }
    
        profile::save();
    }
    else {
        g_powerIsUp = false;
        profile::saveImmediately();
        g_powerIsUp = true;

        profile::enableSave(false);
        powerDown();
        profile::enableSave(true);

        g_testPowerUpDelay = true;
        g_powerDownTime = millis();
    }

    return true;
}

void powerDownBySensor() {
    if (g_powerIsUp) {
        for (int i = 0; i < CH_NUM; ++i) {
            Channel::get(i).outputEnable(false);
        }

        g_powerIsUp = false;
        profile::saveImmediately();
        g_powerIsUp = true;

        profile::enableSave(false);
        powerDown();
        profile::enableSave(true);
    }
}

////////////////////////////////////////////////////////////////////////////////

bool reset() {
    if (psuReset()) {
        profile::save();
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

static void disableChannels() {
    for (int i = 0; i < CH_NUM; ++i) {
        if (Channel::get(i).isOutputEnabled()) {
            Channel::get(i).outputEnable(false);
        }
    }
}

void onProtectionTripped() {
    if (isPowerUp()) {
        if (persist_conf::isShutdownWhenProtectionTrippedEnabled()) {
            if (g_insideInterruptHandler) {
                g_shutdownOnNextTick = true;
                disableChannels();
            } else {
                powerDownBySensor();
            }
        } else {
            if (persist_conf::isOutputProtectionCoupleEnabled()) {
                disableChannels();
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void tick() {
    ++g_mainLoopCounter;

    if (g_shutdownOnNextTick) {
        g_shutdownOnNextTick = false;
        powerDownBySensor();
    }

	uint32_t tick_usec = micros();

#if CONF_DEBUG
    debug::tick(tick_usec);
#endif

	g_powerOnTimeCounter.tick(tick_usec);

	temperature::tick(tick_usec);

	fan::tick(tick_usec);

    ////dummy eeprom read
    //uint8_t buf[128];
    //eeprom::read(buf, 128, 512);

    for (int i = 0; i < CH_NUM; ++i) {
        Channel::get(i).tick(tick_usec);
    }

    trigger::tick(tick_usec);

    list::tick(tick_usec);

    // if we move this, for example, after ethernet::tick we could get
    // (in certain situations, see #25) PWRGOOD error on channel after
    // the "pow:syst 1" command is executed 
	sound::tick(tick_usec);

    profile::tick(tick_usec);

    serial::tick(tick_usec);

#if OPTION_ETHERNET
    if (g_mainLoopCounter % 2 == 0) {
        // tick ethernet every other time
	    ethernet::tick(tick_usec);
    }
#endif

    scpi::tick(tick_usec);
    
#if OPTION_DISPLAY
#ifdef EEZ_PSU_SIMULATOR
    if (simulator::front_panel::isOpened()) {
#endif
        gui::touch::tick(tick_usec);
        gui::touchHandling(tick_usec);

        if (g_mainLoopCounter % 2 == 1) {
            // tick gui every other time
            tick_usec = micros();
            gui::tick(tick_usec);
        }
#ifdef EEZ_PSU_SIMULATOR
    }
#endif
#endif

	event_queue::tick(tick_usec);

#if (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12) && OPTION_SYNC_MASTER && !defined(EEZ_PSU_SIMULATOR)
	updateMasterSync();
#endif

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
#if OPTION_WATCHDOG
	watchdog::tick(tick_usec);
#endif
#endif
}

void criticalTick() {
    if (!g_powerIsUp) {
        return;
    }

    uint32_t tick_usec = micros();

    static uint32_t lastTick = 0;

    if (lastTick == 0) {
        lastTick = tick_usec;
    } else if (tick_usec - lastTick >= (uint32_t)(list::isActive() ? 250 : ADC_READ_TIME_US / 2)) {
        lastTick = tick_usec;

        for (int i = 0; i < CH_NUM; ++i) {
            Channel::get(i).tick(tick_usec);
        }
        list::tick(tick_usec);
    }

#if OPTION_DISPLAY
    static uint32_t lastTickTouch = 0;

#ifdef EEZ_PSU_SIMULATOR
    if (simulator::front_panel::isOpened()) {
#endif
        if (lastTickTouch == 0) {
            lastTickTouch = tick_usec;
        } else if (tick_usec - lastTickTouch >= 5000) {
            lastTickTouch = tick_usec;
            gui::touch::tick(tick_usec);
            gui::touchHandling(tick_usec);
        }
#ifdef EEZ_PSU_SIMULATOR
    }
#endif

#endif
}

////////////////////////////////////////////////////////////////////////////////

void setEsrBits(int bit_mask) {
    SCPI_RegSetBits(&serial::scpi_context, SCPI_REG_ESR, bit_mask);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        SCPI_RegSetBits(&ethernet::scpi_context, SCPI_REG_ESR, bit_mask);
	}
#endif
}


void setQuesBits(int bit_mask, bool on) {
    reg_set_ques_bit(&serial::scpi_context, bit_mask, on);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        reg_set_ques_bit(&ethernet::scpi_context, bit_mask, on);
	}
#endif
}

void generateError(int16_t error) {
    SCPI_ErrorPush(&serial::scpi_context, error);
#if OPTION_ETHERNET
	if (ethernet::g_testResult == TEST_OK) {
        SCPI_ErrorPush(&ethernet::scpi_context, error);
    }
#endif
	event_queue::pushEvent(error);
}

////////////////////////////////////////////////////////////////////////////////

void SPI_usingInterrupt(uint8_t interruptNumber) {
#if REPLACE_SPI_TRANSACTIONS_IMPLEMENTATION
#else
    SPI.usingInterrupt(interruptNumber);
#endif
}

void SPI_beginTransaction(SPISettings &settings) {
#if REPLACE_SPI_TRANSACTIONS_IMPLEMENTATION
    noInterrupts();

    if (&settings == &MCP23S08_SPI)      { SPI.setClockDivider(SPI_CLOCK_DIV4); SPI.setBitOrder(MSBFIRST); SPI.setDataMode(SPI_MODE0); }
    else if (&settings == &DAC8552_SPI)  { SPI.setClockDivider(SPI_CLOCK_DIV4); SPI.setBitOrder(MSBFIRST); SPI.setDataMode(SPI_MODE1); }
    else if (&settings == &ADS1120_SPI)  { SPI.setClockDivider(SPI_CLOCK_DIV4); SPI.setBitOrder(MSBFIRST); SPI.setDataMode(SPI_MODE1); }
    else if (&settings == &TLC5925_SPI)  { SPI.setClockDivider(SPI_CLOCK_DIV4); SPI.setBitOrder(MSBFIRST); SPI.setDataMode(SPI_MODE0); }
    else if (&settings == &PCA21125_SPI) { SPI.setClockDivider(SPI_CLOCK_DIV4); SPI.setBitOrder(MSBFIRST); SPI.setDataMode(SPI_MODE0); }
    else if (&settings == &AT25256B_SPI) { SPI.setClockDivider(SPI_CLOCK_DIV4); SPI.setBitOrder(MSBFIRST); SPI.setDataMode(SPI_MODE0); }
#if defined(EEZ_PSU_ARDUINO_DUE)
    else if (&settings == &ETHERNET_SPI) { SPI.setClockDivider(10);             SPI.setBitOrder(MSBFIRST); SPI.setDataMode(SPI_MODE0); }
#else
    else if (&settings == &ETHERNET_SPI) { SPI.setClockDivider(SPI_CLOCK_DIV2); SPI.setBitOrder(MSBFIRST); SPI.setDataMode(SPI_MODE0); }
#endif
#else
    SPI.beginTransaction(settings);
#endif
}

void SPI_endTransaction() {
#if REPLACE_SPI_TRANSACTIONS_IMPLEMENTATION
    interrupts();
#else
    SPI.endTransaction();
#endif
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
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
    return "Arduino, R5B12";
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
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
    return "W5500";
#endif
#else
    return "Simulator";
#endif
}

void enterTimeCriticalMode() {
    g_isTimeCriticalMode = true;
}

bool isTimeCriticalMode() {
    return g_isTimeCriticalMode;
}

void leaveTimeCriticalMode() {
    g_isTimeCriticalMode = false;
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

bool isFrontPanelLocked() {
    return g_rlState != RL_STATE_LOCAL;
}

#if (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12) && OPTION_SYNC_MASTER && !defined(EEZ_PSU_SIMULATOR)
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