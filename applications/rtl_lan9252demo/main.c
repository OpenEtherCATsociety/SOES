#include <stdio.h>
#include "ecat_slv.h"
#include "utypes.h"

/* Application variables */
_Objects    Obj;

void cb_get_inputs (void)
{
}

void cb_set_outputs (void)
{
}

int main_run (void * arg)
{
   static esc_cfg_t config =
   {
      .user_arg = "/spi1/lan9252",
      .use_interrupt = 0,
      .watchdog_cnt = 150,
      .set_defaults_hook = NULL,
      .pre_state_change_hook = NULL,
      .post_state_change_hook = NULL,
      .application_hook = NULL,
      .safeoutput_override = NULL,
      .pre_object_download_hook = NULL,
      .post_object_download_hook = NULL,
      .rxpdo_override = NULL,
      .txpdo_override = NULL,
      .esc_hw_interrupt_enable = NULL,
      .esc_hw_interrupt_disable = NULL,
      .esc_hw_eep_handler = NULL,
      .esc_check_dc_handler = NULL,
   };

   ecat_slv_init (&config);

   while (1)
   {
      ecat_slv();
   }

   return 0;
}

int main (void)
{
   rprintf ("Hello Main\n");
   task_spawn ("soes", main_run, 8, 2048, NULL);
}
