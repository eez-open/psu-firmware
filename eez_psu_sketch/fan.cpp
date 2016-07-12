/*
 * EEZ PSU Firmware
 * Copyright (C) 2016-present, Envox d.o.o.
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

#define FAN_NOMINAL_RPM 4500 // nazivni broj okretaja u minuti za PWM=255

#define FAN_MIN_TEMP 45 // temperatura ukljuèivanja fana (45oC)
#define FAN_MAX_TEMP 75 // max. dopuštena temperatura (75oC) koja ako ostane više od FAN_MAX_TEMP_DELAY sekundi gasi se glavno napajanje
#define FAN_MIN_PWM 12 //  PWM vrijednost za min. brzinu (12) 
#define FAN_MAX_PWM 255 // PWM vrijednost za max. brzinu (255)

#define FAN_ERR_CURRENT 1 //  max. dozvoljena izlazna struja u amperima ako je fan ili temp. senzor u kvaru
#define FAN_MAX_TEMP_DELAY 30 // definira nakon koliko sekundi æe se gasiti glavno napajanje
#define FAN_MAX_TEMP_DROP 15 // definira koliko stupnjeva mora pasti temperatura ispod FAN_MAX_TEMP da bi se ponovno moglo upaliti glavno napajanje. Prijevremeni pokušaj paljenja javljati æe grešku -200

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6

#include "fan.h"

namespace eez {
namespace psu {
namespace fan {

TestResult test_result = psu::TEST_FAILED;

static int fan_speed_pwm = 0;
static int rpm = 0;

static unsigned long test_start_time;

////////////////////////////////////////////////////////////////////////////////

int dt_to_rpm(unsigned long dt) {
	dt *= 2; // duty cycle is 50%
	dt *= 2; // 2 impulse per revolution
	return (int)(60L * 1000 * 1000 / dt);
}

unsigned long rpm_to_dt(int rpm) {
	unsigned long dt = 60L * 1000 * 1000 / rpm;
	dt /= 2;
	dt /= 2;
	return dt;
}

int pwm_to_rpm(int pwm) {
	return (int)util::remap((float)pwm, FAN_MIN_PWM, 0, FAN_MAX_PWM, FAN_NOMINAL_RPM);
}

////////////////////////////////////////////////////////////////////////////////

enum RpmMeasureState {
	RPM_MEASURE_STATE_INIT,
	RPM_MEASURE_STATE_T1,
	RPM_MEASURE_STATE_T2,
	RPM_MEASURE_STATE_FINISHED,
};

static int rpm_measure_interrupt_number;
static RpmMeasureState rpm_measure_state = RPM_MEASURE_STATE_FINISHED;
static unsigned long rpm_measure_t1;
static unsigned long rpm_measure_t2;

void finish_rpm_measure();
void rpm_measure_interrupt_handler();

void start_rpm_measure() {
	rpm_measure_state = RPM_MEASURE_STATE_INIT;
	rpm_measure_t1 = 0;
	rpm_measure_t2 = 0;

	analogWrite(FAN_PWM, FAN_MAX_PWM);
	delay(2);
	attachInterrupt(rpm_measure_interrupt_number, rpm_measure_interrupt_handler, CHANGE);

#ifdef EEZ_PSU_SIMULATOR
	rpm_measure_t1 = 0;
	rpm_measure_t2 = rpm_to_dt(pwm_to_rpm(analogRead(FAN_PWM)));
	rpm_measure_state = RPM_MEASURE_STATE_FINISHED;
	finish_rpm_measure();
#endif
}

void rpm_measure_interrupt_handler() {
	int x = digitalRead(FAN_SENSE);
	if (rpm_measure_state == RPM_MEASURE_STATE_INIT && x) {
		rpm_measure_state = RPM_MEASURE_STATE_T1;
	} else if (rpm_measure_state == RPM_MEASURE_STATE_T1 && !x) {
		rpm_measure_t1 = micros();
		rpm_measure_state = RPM_MEASURE_STATE_T2;
	} else if (rpm_measure_state == RPM_MEASURE_STATE_T2 && x) {
		rpm_measure_t2 = micros();
		rpm_measure_state = RPM_MEASURE_STATE_FINISHED;
		finish_rpm_measure();
	}
}

void finish_rpm_measure() {
	detachInterrupt(rpm_measure_interrupt_number);
	analogWrite(FAN_PWM, fan_speed_pwm);

	if (rpm_measure_state == RPM_MEASURE_STATE_FINISHED) {
		rpm = dt_to_rpm(rpm_measure_t2 - rpm_measure_t1);
	} else {
		rpm_measure_state = RPM_MEASURE_STATE_FINISHED;
		rpm = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////

bool init() {
	rpm_measure_interrupt_number = digitalPinToInterrupt(FAN_SENSE);

	return test();
}

void test_start() {
	if (OPTION_FAN) {
		analogWrite(FAN_PWM, FAN_MAX_PWM);
		test_start_time = millis();
	}
}

bool test() {
	if (OPTION_FAN) {
		unsigned long time_since_test_start = millis() - test_start_time;
		if (time_since_test_start < 250) {
			delay(300 - time_since_test_start);
		}

		start_rpm_measure();

		for (int i = 0; i < 25 && rpm_measure_state != RPM_MEASURE_STATE_FINISHED; ++i) {
			delay(1);
		}

		if (rpm_measure_state != RPM_MEASURE_STATE_FINISHED) {
			finish_rpm_measure();
			test_result = psu::TEST_FAILED;
		} else {
			test_result = psu::TEST_OK;
			DebugTraceF("Fan RPM: %d", rpm);
		}
	} else {
		test_result = psu::TEST_SKIPPED;
	}

	if (test_result == psu::TEST_FAILED) {
		psu::generateError(SCPI_ERROR_FAN_TEST_FAILED);
	}

	return test_result != psu::TEST_FAILED;
}

void tick(unsigned long tick_usec) {
}

}
}
} // namespace eez::psu::fan

#endif