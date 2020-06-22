/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#ifndef __ECAT_SLV_H__
#define __ECAT_SLV_H__

#include "options.h"
#include "esc.h"

/**
 * This function is called when to get input values
 */
void cb_get_inputs();

/**
* This function is called when to set outputs values
 */
void cb_set_outputs();

/** Set the watchdog count value
 *
 * @param[in] watchdogcnt  = new watchdog count value
 */
void APP_setwatchdog (int watchdogcnt);

#define DIG_PROCESS_INPUTS_FLAG     0x01
#define DIG_PROCESS_OUTPUTS_FLAG    0x02
#define DIG_PROCESS_WD_FLAG         0x04
#define DIG_PROCESS_APP_HOOK_FLAG   0x08
/** Implements the watch-dog counter to count if we should make a state change
 * due to missing incoming SM2 events. Updates local I/O and run the application
 * in the following order, call read EtherCAT outputs, execute user provided
 * application hook and call write EtherCAT inputs.
 *
 * @param[in]   flags     = User input what to execute
 */
void DIG_process (uint8_t flags);

/**
 * Handler for SM change, SM0/1, AL CONTROL and EEPROM events, the application
 * control what interrupts that should be served and re-activated with
 * event mask argument
 *
 * @param[in]   event_mask = Event mask for interrupts to serve and re-activate
 *                           after served
 */
void ecat_slv_worker (uint32_t event_mask);

/**
 * Poll SM0/1, EEPROM and AL CONTROL events in a SM/DC synchronization
 * application
 */
void ecat_slv_poll (void);

/**
 * Poll all events in a free-run application
 */
void ecat_slv (void);

/**
 * Initialize the slave stack
 *
 * @param[in]   config     = User input how to configure the stack
 */
void ecat_slv_init (esc_cfg_t * config);

#endif /* __ECAT_SLV_H__ */
