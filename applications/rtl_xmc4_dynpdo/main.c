/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <kern.h>
#include "esc.h"
#include "esc_hw.h"
#include "ecat_slv.h"
#include "options.h"
#include "utypes.h"
#include "bsp.h"

/* Application variables */
_Objects    Obj;

/*
 * This function is called to get input values
 */
void cb_get_inputs()
{
   Obj.Buttons.Button1 = gpio_get (GPIO_BUTTON1);
}

/*
 * This function is called to set output values
 */
void cb_set_outputs()
{
   gpio_set (GPIO_LED1, Obj.LEDgroup0.LED0);
   gpio_set (GPIO_LED2, Obj.LEDgroup1.LED1);
}

void cb_state_change (uint8_t * as, uint8_t * an)
{
   if (*as == SAFEOP_TO_OP)
   {
      /* Enable watchdog interrupt */
      ESC_ALeventmaskwrite (ESC_ALeventmaskread() | ESCREG_ALEVENT_WD);
   }
   else if (*as == PREOP_TO_SAFEOP)
   {
      /* Write initial input data requried if an input only slave,
       * otherwise the SM3 will never occur.
       */
      DIG_process (DIG_PROCESS_INPUTS_FLAG);
   }
}

/* Setup of DC */
uint16_t dc_checker (void)
{
   /* Indicate we run DC */
   ESCvar.dcsync = 1;
   /* Fetch the sync counter limit (SDO10F1) */
   ESCvar.synccounterlimit = Obj.ErrorSettings.SyncErrorCounterLimit;
   return 0;
}

int main (void)
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
      .esc_check_dc_handler = dc_checker,
      .get_device_id = NULL
   };

   rprintf ("Hello world\n");
   ecat_slv_init (&config);
   return 0;
}
