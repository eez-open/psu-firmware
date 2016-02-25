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

#include "persist_conf.h"
#include "ioexp.h"
#include "adc.h"
#include "dac.h"

#define IS_OVP_VALUE(channel, cpv) (&cpv == &channel->ovp)
#define IS_OCP_VALUE(channel, cpv) (&cpv == &channel->ocp)
#define IS_OPP_VALUE(channel, cpv) (&cpv == &channel->opp)

namespace eez {
namespace psu {

/// PSU channel.
class Channel {
public:
    /// Binary flags for the channel calibration configuration.
    struct CalibrationConfigurationFlags {
        /// Is voltage calibrated?
        unsigned u_cal_params_exists : 1; 
        /// Is current calibrated?
        unsigned i_cal_params_exists : 1;
    };

    /// Calibration parameters for the single point.
    struct CalibrationValuePointConfiguration {
        /// Value set on DAC by the calibration module.
        float dac;
        /// Real value, in volts, set by the user who reads it on the instrument (voltmeter and ampermeter).
        float val;
        /// Value read from ADC.
        float adc;

    };

    /// Calibration parameters for the voltage and current.
    /// There are three points defined: `min`, `mid` and `max`.
    /// Only `min` and `max` are used in actual calculations -
    /// `mid` is only used for the validity checking.
    /// Here is how `DAC` value is calculated from the `real_value` set by user:
    /// `DAC = min.dac + (real_value - min.val) * (max.dac - min.dac) / (max.val - min.val);`
    /// And here is how `real_value` is calculated from the `ADC` value:
    /// `real_value = min.val + (ADC - min.adc) * (max.val - min.val) / (max.adc - min.adc);`
    struct CalibrationValueConfiguration {
        /// Min point.
        CalibrationValuePointConfiguration min;
        /// Mid point.
        CalibrationValuePointConfiguration mid;
        /// Max point.
        CalibrationValuePointConfiguration max;
    };

    /// A structure where calibration parameters for the channel are stored.
    struct CalibrationConfiguration {
        /// Used by the persist_conf.
        persist_conf::BlockHeader header;

        /// Flags
        CalibrationConfigurationFlags flags;

        /// Calibration parameters for the voltage.
        CalibrationValueConfiguration u; 

        /// Calibration parameters for the current.
        CalibrationValueConfiguration i;

        /// Date when calibration is saved.
        /// Automatically set if RTC is present.
        /// Format is YYYYMMDD. 
        char calibration_date[8 + 1];

        /// Remark about calibration set by user.
        char calibration_remark[CALIBRATION_REMARK_MAX_LENGTH + 1];
    };

    /// Binary flags for the channel protection configuration
    struct ProtectionConfigurationFlags {
        /// Is OVP enabled?
        unsigned u_state : 1;
        /// Is OCP enabled?
        unsigned i_state : 1;
        /// Is OPP enabled?
        unsigned p_state : 1;
    };

    /// Channel OVP, OVP and OPP configuration parameters like level and delay.
    struct ChannelProtectionConfiguration {
        persist_conf::BlockHeader header;

        ProtectionConfigurationFlags flags;

        float u_delay;
        float i_delay;
        float p_delay;
        float p_level;
    };

    /// Channel binary flags like output enabled, sense enabled, ...
    struct Flags {
        unsigned output_enabled : 1;
        unsigned dp_on : 1;
        unsigned sense_enabled : 1;
        unsigned cv_mode : 1;
        unsigned cc_mode : 1;
        unsigned power_ok : 1;
        unsigned cal_enabled : 1;
    };

    /// Voltage and current data set and measured during runtime.
    struct Value {
        float set;
        float mon_dac;
        float mon;
        float step;

        void init(float def_step);
    };

    /// Runtime protection binary flags (alarmed, tripped)
    struct ProtectionFlags {
        unsigned alarmed : 1;
        unsigned tripped : 1;
    };

    /// Runtime protection values    
    struct ProtectionValue {
        ProtectionFlags flags;
        uint32_t alarm_started;
    };

#ifdef EEZ_PSU_SIMULATOR
    /// Per channel simulator data
    struct Simulator {
        bool oe;
        bool load_enabled;
        float load;
        float u_set;
        float u_dac;
        float i_set;
        float i_dac;
        float temperature[temp_sensor::COUNT];

        void setLoadEnabled(bool value);
        bool getLoadEnabled();

        void setLoad(float value);
        float getLoad();
    };
#endif // EEZ_PSU_SIMULATOR

    /// Get channel instance
    /// \param channel_index Zero based channel index, greater then or equal to 0 and less then CH_MAX.
    /// \returns Reference to channel.
    static Channel &get(int channel_index);

    /// Channel index. Starts from 1.
    int8_t index;

    uint8_t isolator_pin;
    uint8_t ioexp_pin;
    uint8_t convend_pin;
    uint8_t adc_pin;
    uint8_t dac_pin;
    uint16_t bp_led_out_plus;
    uint16_t bp_led_out_minus;
    uint16_t bp_led_sense_plus;
    uint16_t bp_led_sense_minus;
    uint16_t bp_relay_sense;
    uint8_t cc_led_pin;
    uint8_t cv_led_pin;

    float U_MIN;
    float U_DEF;
    float U_MAX;
    float U_MIN_STEP;
    float U_DEF_STEP;
    float U_MAX_STEP;
    float U_CAL_VAL_MIN;
    float U_CAL_VAL_MID;
    float U_CAL_VAL_MAX;
    float U_CURR_CAL; // voltage level during current calibration

    bool OVP_DEFAULT_STATE;
    float OVP_MIN_DELAY;
    float OVP_DEFAULT_DELAY;
    float OVP_MAX_DELAY;

    float I_MIN;
    float I_DEF;
    float I_MAX;
    float I_MIN_STEP;
    float I_DEF_STEP;
    float I_MAX_STEP;
    float I_CAL_VAL_MIN;
    float I_CAL_VAL_MID;
    float I_CAL_VAL_MAX;
    float I_VOLT_CAL; // current level during voltage calibration

    bool OCP_DEFAULT_STATE;
    float OCP_MIN_DELAY;
    float OCP_DEFAULT_DELAY;
    float OCP_MAX_DELAY;

    bool OPP_DEFAULT_STATE;
    float OPP_MIN_DELAY;
    float OPP_DEFAULT_DELAY;
    float OPP_MAX_DELAY;
    float OPP_MIN_LEVEL;
    float OPP_DEFAULT_LEVEL;
    float OPP_MAX_LEVEL;

    IOExpander ioexp;
    AnalogDigitalConverter adc;
    DigitalAnalogConverter dac;

    Flags flags;

    Value u;
    Value i;

    CalibrationConfiguration cal_conf;
    ChannelProtectionConfiguration prot_conf;

    ProtectionValue ovp;
    ProtectionValue ocp;
    ProtectionValue opp;

#ifdef EEZ_PSU_SIMULATOR
    Simulator simulator;
#endif // EEZ_PSU_SIMULATOR

    Channel(
        int8_t index,
        uint8_t isolator_pin, uint8_t ioexp_pin, uint8_t convend_pin, uint8_t adc_pin, uint8_t dac_pin,
        uint16_t bp_led_out_plus, uint16_t bp_led_out_minus, uint16_t bp_led_sense_plus, uint16_t bp_led_sense_minus, uint16_t bp_relay_sense,
        uint8_t cc_led_pin, uint8_t cv_led_pin,
        float U_MIN, float U_DEF, float U_MAX, float U_MIN_STEP, float U_DEF_STEP, float U_MAX_STEP, float U_CAL_VAL_MIN, float U_CAL_VAL_MID, float U_CAL_VAL_MAX, float U_CURR_CAL,
        bool OVP_DEFAULT_STATE, float OVP_MIN_DELAY, float OVP_DEFAULT_DELAY, float OVP_MAX_DELAY,
        float I_MIN, float I_DEF, float I_MAX, float I_MIN_STEP, float I_DEF_STEP, float I_MAX_STEP, float I_CAL_VAL_MIN, float I_CAL_VAL_MID, float I_CAL_VAL_MAX, float I_VOLT_CAL,
        bool OCP_DEFAULT_STATE, float OCP_MIN_DELAY, float OCP_DEFAULT_DELAY, float OCP_MAX_DELAY,
        bool OPP_DEFAULT_STATE, float OPP_MIN_DELAY, float OPP_DEFAULT_DELAY, float OPP_MAX_DELAY, float OPP_MIN_LEVEL, float OPP_DEFAULT_LEVEL, float OPP_MAX_LEVEL);

    /// Initialize channel and underlying hardware.
    /// Makes a required tests, for example ADC, DAC and IO Expander tests.
    bool init();

    /// Reset the channel to default values.
    void reset();

    /// Clear channel calibration configuration.
    void clearCalibrationConf();

    /// Test the channel.
    bool test();

    /// Is channel power ok (state of PWRGOOD bit in IO Expander)?
    bool isPowerOk();
    
    /// Is channel test failed?
    bool isTestFailed();
    
    /// Is channel test ok?
    bool isTestOk();

    /// Is channel ready to work with?
    bool isOk();

    /// Called by main loop, used for channel maintenance.
    void tick(unsigned long tick_usec);

    /// Called from IO expander interrupt routine.
    /// @param gpio State of IO expander GPIO register.
    /// @param adc_data ADC snapshot data.
    void event(uint8_t gpio, int16_t adc_data);

    /// Called when device power is turned off, so channel
    /// can do its own housekeeping.
    void onPowerDown();

    /// Force ADC read of u.mon_dac and i.mon_dac.
    void adcReadMonDac();

    /// Force ADC read of all values: u.mon, u.mon_dac, i.mon and i.mon_dac.
    void adcReadAll();

    /// Force update of all channel state (u.set, i.set, output enable, remote sensing, ...).
    /// This is called when channel is recovering from hardware failure.
    void update();

    /// Force update of only output enable state, i.e. enable/disable output depending of output_enabled flag.
    void updateOutputEnable();

    /// Enable/disable channel output.
    void outputEnable(bool enable);

    /// Is channel output enabled?
    bool isOutputEnabled();

    /// Enable/disable remote sensing.
    void remoteSensingEnable(bool enable);

    /// Is remote sensing enabled?
    bool isRemoteSensingEnabled();

    /// Set channel voltage level.
    void setVoltage(float voltage);

    /// Set channel current level
    void setCurrent(float current);

    /// Is channel calibrated, both voltage and current?
    bool isCalibrationExists();

    /// Is OVP, OCP or OPP tripped?
    bool isTripped();

    /// Clear channel protection tripp state.
    void clearProtection();

    /// Turn on/off bit in SCPI Questinable Instrument Isummary register for this channel.
    void setQuesBits(int bit_mask, bool on);

    /// Turn on/off bit in SCPI Operational Instrument Isummary register for this channel.
    void setOperBits(int bit_mask, bool on);

    /// Is channel in CV (constant voltage) mode?
    bool isCvMode() { return flags.cv_mode && !flags.cc_mode; }

    /// Is channel in CC (constant current) mode?
    bool isCcMode() { return flags.cc_mode && !flags.cv_mode; }

    /// Returns "CC", "CV" or "UR"
    char *getCvModeStr();

    /// Remap ADC data value to actual voltage value (use calibration if configured).
    float remapAdcDataToVoltage(int16_t adc_data);

    /// Remap ADC data value to actual current value (use calibration if configured).
    float remapAdcDataToCurrent(int16_t adc_data);

    /// Remap voltage value to ADC data value (use calibration if configured).
    int16_t remapVoltageToAdcData(float value);

    /// Remap current value to ADC data value (use calibration if configured).
    int16_t remapCurrentToAdcData(float value);

private:
    bool delayed_dp_off;
    uint32_t delayed_dp_off_start;

    void clearProtectionConf();
    void protectionEnter(ProtectionValue &cpv);
    void protectionCheck(ProtectionValue &cpv);
    float readingToCalibratedValue(Value *cv, float mon_reading);
    void valueAddReading(Value *cv, float value);
    void valueAddReadingDac(Value *cv, float value);
    void adcDataIsReady(int16_t data);
    void setCcMode(bool cc_mode);
    void setCvMode(bool cv_mode);
    void updateBoardCcAndCvSwitch();
    void doOutputEnable(bool enable);
    void doRemoteSensingEnable(bool enable);
    void doDpEnable(bool enable);
};

}
} // namespace eez::psu
