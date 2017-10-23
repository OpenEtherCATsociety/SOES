#ifndef __SLAVE_H__
#define __SLAVE_H__

#include "utypes.h"

/**
 * This function gets input values and updates Rb.Buttons
 */
void cb_get_Buttons();

/**
 * This function sets output values according to Wb.LEDgroup0
 */
void cb_set_LEDgroup0();

/**
 * This function sets output values according to Wb.LEDgroup1
 */
void cb_set_LEDgroup1();

/**
 * This function is called after a SDO write of the object Cb.variableRW.
 */
void cb_post_write_variableRW(int subindex);

/**
 * Used as ISR for SM0/1, EEPROM and AL CONTROL events in a SM/DC
 * synchronization application
 */
void ecat_slv_isr (void);

/**
 * Used to poll SM0/1, EEPROM and AL CONTROL events in a SM/DC synchronization
 * application
 */
void ecat_slv_poll (void);

/**
 * Used to poll all events in a free-run application
 */
void ecat_slv (void);

/**
 * Initialize the slave stack
 * \code
 * Configuration parameters for the slave stack
 * static esc_cfg_t config =
 * {
 *   .use_interrupt = 1,   Flag to the stack we'll use interrupts for
 *                         SM2 and DC
 *   .user_arg = NULL,     User define ARG (optional)
 *   .watchdog_cnt = 100,  Non UNIT watchdog counter, for the application
 *                         developer to decide UNINT. This example set 100
 *                         cnt and by calling ecat_slv or
 *                         DIG_process(DIG_PROCESS_WD_FLAG) every 1ms,
 *                         it create an watchdog running at ~100ms.
 *
 *   Values taken from config.h to configure the stack
 *   Mailbox In/Out
 *  .mb[0] = {MBX0_sma, MBX0_sml, MBX0_sme, MBX0_smc, 0},
 *  .mb[1] = {MBX1_sma, MBX1_sml, MBX1_sme, MBX1_smc, 0},
 *   BOOT Mailbox In/Out
 *  .mb_boot[0] = {MBX0_sma_b, MBX0_sml_b, MBX0_sme_b, MBX0_smc_b, 0},
 *  .mb_boot[1] = {MBX1_sma_b, MBX1_sml_b, MBX1_sme_b, MBX1_smc_b, 0},
 *   PDO SyncManager 2 and 3
 *  .pdosm[0] = {SM2_sma, 0, 0, SM2_smc, SM2_act},
 *  .pdosm[1] = {SM3_sma, 0, 0, SM3_smc, SM3_act},*
 *
 *  .pre_state_change_hook = NULL, Hook called before state transition
 *  .post_state_change_hook = NULL, Hook called after state transition
 *  .application_hook = NULL, Callback in application loop called when
 *                                DIG_process(DIG_PROCESS_APP_HOOK_FLAG)
 *  .safeoutput_override = NULL, User override of default safeoutput
 *                                           when stack stop outputs
 *  .pre_object_download_hook = NULL, Hook called before object download, it hook
 *                                    return 0 the download will not take place
 *  .post_object_download_hook = NULL, Hook called after object download
 *  .rxpdo_override = NULL, User override of default rxpdo
 *  .txpdo_override = NULL , User override of default txpdo
 *  .esc_interrupt_enable = NULL, Callback to function that enable IRQ based
 *                                on the Event MASK.
 *  .esc_interrupt_disable = NULL Callback to function that disable IRQ based
 *                                on the Event MASK.
 *  .esc_hw_eep_handler = NULL Callback to function that handle an emulated
 *                             eeprom.
 * };
 * \endcode
 */
void ecat_slv_init (esc_cfg_t * config);

#endif /* __SLAVE_H__ */
