
#include <cc.h>
#include <kern.h>
#include <xmc4.h>
#include <bsp.h>
#include "esc_hw.h"
#include "esc.h"
#include "ecat_slv.h"
#include "config.h"
#include "utypes.h"

/**
 * This function is called when to get input values
 */
void cb_get_inputs()
{
 Rb.Buttons.Button1 = gpio_get(GPIO_BUTTON1);
}

/**
* This function is called when to set outputs values
 */
void cb_set_outputs()
{
   gpio_set(GPIO_LED1, Wb.LEDgroup0.LED0);
   gpio_set(GPIO_LED2, Wb.LEDgroup1.LED1);
}

/** Optional: Hook called after state change for application specific
 * actions for specific state changes.
 */
void post_state_change_hook (uint8_t * as, uint8_t * an)
{
   /* Add specific step change hooks here */
   if ((*as == SAFEOP_TO_OP))
   {
      ESC_ALeventmaskwrite(ESC_ALeventmaskread() | ESCREG_ALEVENT_WD);
   }
}

void safe_out(void)
{
   rprintp("safe out\n");
   return;
}

/* Setup of DC */
uint16_t dc_checker(void)
{
   /* Indicate we run DC */
   ESCvar.dcsync = 1;
   /* Fetch the sync counter limit  SDO10F1*/
   ESCvar.synccounterlimit = Mb.ErrorSettings.SyncErrorCounterLimit;
   return 0;
}

/* Configuration parameters for SOES
 * SM and Mailbox parameters comes from the
 * generated config.h
 */
int main(void)
{
   esc_cfg_t config =
   {
      .user_arg = NULL,
      .use_interrupt = 1,
      .watchdog_cnt = INT32_MAX, /* Use HW SM watchdog instead */
      .mbxsize = MBXSIZE,
      .mbxsizeboot = MBXSIZEBOOT,
      .mbxbuffers = MBXBUFFERS,
      .rxpdosaddress = &Wb,
      .txpdosaddress = &Rb,
      .mb[0] = {MBX0_sma, MBX0_sml, MBX0_sme, MBX0_smc, 0},
      .mb[1] = {MBX1_sma, MBX1_sml, MBX1_sme, MBX1_smc, 0},
      .mb_boot[0] = {MBX0_sma_b, MBX0_sml_b, MBX0_sme_b, MBX0_smc_b, 0},
      .mb_boot[1] = {MBX1_sma_b, MBX1_sml_b, MBX1_sme_b, MBX1_smc_b, 0},
      .pdosm[0] = {SM2_sma, 0, 0, SM2_smc, SM2_act},
      .pdosm[1] = {SM3_sma, 0, 0, SM3_smc, SM3_act},
      .pre_state_change_hook = NULL,
      .post_state_change_hook = post_state_change_hook,
      .application_hook = NULL,
      .safeoutput_override = safe_out,
      .pre_object_download_hook = NULL,
      .post_object_download_hook = NULL,
      .rxpdo_override = NULL,
      .txpdo_override = NULL,
      .esc_hw_interrupt_enable = ESC_interrupt_enable,
      .esc_hw_interrupt_disable = ESC_interrupt_disable,
      .esc_hw_eep_handler = ESC_eep_handler,
      .esc_check_dc_handler = dc_checker
   };

   rprintf("Hello Main\n");
   ecat_slv_init(&config);
   return 0;
}
