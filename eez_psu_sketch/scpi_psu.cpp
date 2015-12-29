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
#include "scpi_psu.h"

#include "scpi_appl.h"
#include "scpi_cal.h"
#include "scpi_core.h"
#include "scpi_debug.h"
#include "scpi_diag.h"
#include "scpi_inst.h"
#include "scpi_meas.h"
#include "scpi_mem.h"
#include "scpi_outp.h"
#include "scpi_sour.h"
#include "scpi_stat.h"
#include "scpi_syst.h"

#ifdef EEZ_PSU_SIMULATOR 
#include "scpi_simu.h"
#endif

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

#define SCPI_COMMANDS \
    SCPI_APPL_COMMANDS \
    SCPI_CAL_COMMANDS \
    SCPI_CORE_COMMANDS \
    SCPI_DEBUG_COMMANDS \
    SCPI_DIAG_COMMANDS \
    SCPI_INST_COMMANDS \
    SCPI_MEAS_COMMANDS \
    SCPI_MEM_COMMANDS \
    SCPI_OUTP_COMMANDS \
    SCPI_SIMU_COMMANDS \
    SCPI_SOUR_COMMANDS \
    SCPI_STAT_COMMANDS \
    SCPI_SYST_COMMANDS \

#define SCPI_COMMAND(P, C) scpi_result_t C(scpi_t * context);
SCPI_COMMANDS
#undef SCPI_COMMAND

#if USE_64K_PROGMEM_FOR_CMD_LIST

#define SCPI_COMMAND(P, C) static const char C ## _pattern[] PROGMEM = P;
SCPI_COMMANDS
#undef SCPI_COMMAND

#define SCPI_COMMAND(P, C) {C ## _pattern, C},
static const scpi_command_t scpi_commands[] PROGMEM = {
    SCPI_COMMANDS
    SCPI_CMD_LIST_END
};

#else

#define SCPI_COMMAND(P, C) {P, C},
static const scpi_command_t scpi_commands[] = {
    SCPI_COMMANDS
    SCPI_CMD_LIST_END
};

#endif

////////////////////////////////////////////////////////////////////////////////

void init(scpi_t &scpi_context,
    scpi_psu_t &scpi_psu_context,
    scpi_interface_t *interface,
    char *input_buffer,
    size_t input_buffer_length,
    int16_t *error_queue_data,
    int16_t error_queue_size)
{
    SCPI_Init(&scpi_context, scpi_commands, interface, scpi_units_def,
        MANUFACTURER, psu::getModelName(), PSU_SERIAL, FIRMWARE,
        input_buffer, input_buffer_length, error_queue_data, error_queue_size);

    scpi_context.user_context = &scpi_psu_context;
}

void input(scpi_t &scpi_context, char ch) {
    //if (ch < 0 || ch > 127) {
    //    // non ASCII, call parser now
    //    SCPI_Input(&scpi_context, 0, 0);
    //    return;
    //}

    int result = SCPI_Input(&scpi_context, &ch, 1);
    if (result == -1) {
        // TODO we need better buffer overrun handling here

        // call parser now
        SCPI_Input(&scpi_context, 0, 0);

        // input buffer is now empty, feed it
        SCPI_Input(&scpi_context, &ch, 1);
    }
}

}
}
} // namespace eez::psu::scpi