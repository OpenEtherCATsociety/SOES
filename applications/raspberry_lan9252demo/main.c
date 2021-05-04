#include <stdio.h>
#include <string.h>
#include "ecat_options.h"
#include "ecat_slv.h"
#include "utypes.h"

/* Application variables */
_Objects    Obj;

#if !(DYN_PDO_MAPPING)
uint8_t * txpdo = Obj.txpdo;
uint8_t * rxpdo = Obj.rxpdo;
#endif

void cb_get_inputs (void)
{
   #if DYN_PDO_MAPPING
   memcpy(Obj.txpdo,Obj.rxpdo,BYTE_NUM);
   #else
   memcpy(txpdo,rxpdo,BYTE_NUM);
   #endif
}

void cb_set_outputs (void)
{
}

int main_run (void * arg)
{
   static esc_cfg_t config =
   {
      .user_arg = "rpi3,cs0",
      .use_interrupt = 0,
      .watchdog_cnt = 0, /* Use HW SM watchdog instead */
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

   printf ("Start slave init\n");
   ecat_slv_init (&config);
   printf ("Init finished\n");
   while (1)
   {
      ecat_slv();
   }

   return 0;
}

int main (void)
{
   printf ("Hello Main\n");
   main_run (NULL);
   return 0;
}
