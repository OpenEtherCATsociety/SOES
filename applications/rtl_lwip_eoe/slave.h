#ifndef __SLAVE_H__
#define __SLAVE_H__

#include "utypes.h"
#include "esc.h"

/**
 * This function gets input values and updates Rb.Button1
 */
void cb_get_Button1();

/**
 * This function gets input values and updates Rb.Button2
 */
void cb_get_Button2();

/**
 * This function sets output values according to Wb.LEDgroup1
 */
void cb_set_LEDgroup1();

/**
 * This function sets output values according to Wb.LEDgroup2
 */
void cb_set_LEDgroup2();

/**
 * This function sets output values according to Wb.LEDgroup3
 */
void cb_set_LEDgroup3();

/**
 * This function sets output values according to Wb.LEDgroup4
 */
void cb_set_LEDgroup4();

/**
 * This function sets output values according to Wb.LEDgroup5
 */
void cb_set_LEDgroup5();

/**
 * This function is called after a SDO write of the object Cb.Parameters.
 */
void cb_post_write_Parameters(int subindex);

/**
 * This function is called after a SDO write of the object Cb.variableRW.
 */
void cb_post_write_variableRW(int subindex);

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
 * ISR for SM0/1, EEPROM and AL CONTROL events in a SM/DC
 * synchronization application
 */
CC_DEPRECATED void ecat_slv_isr (void);

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

#endif /* __SLAVE_H__ */
