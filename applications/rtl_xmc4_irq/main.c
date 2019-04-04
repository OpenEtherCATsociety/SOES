/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <cc.h>
#include <kern.h>
#include <xmc4.h>
#include <bsp.h>
#include "esc_hw.h"
#include "esc.h"
#include "ecat_slv.h"
#include "config.h"
#include "utypes.h"

/* Application variables */
_Objects    Obj;

/**
 * This function is called when to get input values
 */
void cb_get_inputs()
{
   Obj.Buttons.Button1 = gpio_get(GPIO_BUTTON1);
}

/**
* This function is called when to set outputs values
 */
void cb_set_outputs()
{
   gpio_set(GPIO_LED1, Obj.LEDgroup0.LED0);
   gpio_set(GPIO_LED2, Obj.LEDgroup1.LED1);
}

void safe_out(void)
{
   rprintp("safe out\n");
   return;
}

void cb_state_change (uint8_t * as, uint8_t * an)
{
   if (*as == SAFEOP_TO_OP)
   {
      /* Enable watchdog interrupt */
      ESC_ALeventmaskwrite(ESC_ALeventmaskread() | ESCREG_ALEVENT_WD);
   }
}

/* Setup of DC */
uint16_t dc_checker(void)
{
   /* Indicate we run DC */
   ESCvar.dcsync = 1;
   /* Fetch the sync counter limit  SDO10F1*/
   ESCvar.synccounterlimit = Obj.ErrorSettings.SyncErrorCounterLimit;
   return 0;
}

/* Configuration parameters for SOES
 * SM and Mailbox parameters comes from the
 * generated config.h
 */
int main(void)
{
   static esc_cfg_t config =
   {
      .user_arg = NULL,
      .use_interrupt = 1,
      .watchdog_cnt = INT32_MAX, /* Use HW SM watchdog instead */
      .set_defaults_hook = NULL,
      .pre_state_change_hook = NULL,
      .post_state_change_hook = cb_state_change,
      .application_hook = NULL,
      .safeoutput_override = NULL,
      .pre_object_download_hook = NULL,
      .post_object_download_hook = NULL,
      .rxpdo_override = NULL,
      .txpdo_override = NULL,
      .esc_hw_interrupt_enable = ESC_interrupt_enable,
      .esc_hw_interrupt_disable = ESC_interrupt_disable,
      .esc_hw_eep_handler = ESC_eep_handler,
      .esc_check_dc_handler = NULL
   };

   rprintf("Hello Main\n");
   ecat_slv_init(&config);
   return 0;
}
