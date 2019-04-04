/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <kern.h>
#include "ecat_slv.h"
#include "utypes.h"
#include "bsp.h"
#include "bootstrap.h"

/* Application variables */
_Rbuffer    Rb;
_Wbuffer    Wb;
_Cbuffer    Cb;

uint32_t encoder_scale;
uint32_t encoder_scale_mirror;

void cb_get_inputs (void)
{
   Rb.button = gpio_get(GPIO_BUTTON_SW1);
   //Rb.button = (flash_drv_get_active_swap() && 0x8);
   Cb.reset_counter++;
   Rb.encoder =  Cb.reset_counter;
}

void cb_set_outputs (void)
{
   gpio_set(GPIO_LED_BLUE, Wb.LED & BIT(0));
}

/** Optional: Hook called after state change for application specific
 * actions for specific state changes.
 */
void post_state_change_hook (uint8_t * as, uint8_t * an)
{

   /* Add specific step change hooks here */
   if ((*as == BOOT_TO_INIT) && (*an == ESCinit))
   {
      boot_inithook ();
   }
   else if((*as == INIT_TO_BOOT) && (*an & ESCerror ) == 0)
   {
      init_boothook ();
   }
}

void post_object_download_hook (uint16_t index, uint8_t subindex,
                                uint16_t flags)
{
   switch(index)
   {
      case 0x7100:
      {
         switch (subindex)
         {
            case 0x01:
            {
               encoder_scale_mirror = encoder_scale;
               break;
            }
         }
         break;
      }
      case 0x8001:
      {
         switch (subindex)
         {
            case 0x01:
            {
               Cb.reset_counter = 0;
               break;
            }
         }
         break;
      }
   }
}

void soes (void * arg)
{

   /* Setup config hooks */
   static esc_cfg_t config =
   {
      .user_arg = "/spi0/et1100",
      .use_interrupt = 0,
      .set_defaults_hook = NULL,
      .watchdog_cnt = 1000,
      .pre_state_change_hook = NULL,
      .post_state_change_hook = post_state_change_hook,
      .application_hook = NULL,
      .safeoutput_override = NULL,
      .pre_object_download_hook = NULL,
      .post_object_download_hook = NULL,
      .rxpdo_override = NULL,
      .txpdo_override = NULL,
      .esc_hw_interrupt_enable = NULL,
      .esc_hw_interrupt_disable = NULL,
      .esc_hw_eep_handler = NULL
   };

   ecat_slv_init (&config);

   while (1)
   {
      ecat_slv();
   }
}

uint8_t load1s, load5s, load10s;
void my_cyclic_callback (void * arg)
{
   while (1)
   {
      task_delay(tick_from_ms (20000));
      stats_get_load (&load1s, &load5s, &load10s);
      DPRINT ("%d:%d:%d (1s:5s:10s)\n",
               load1s, load5s, load10s);
      DPRINT ("Local bootstate: %d App.state: %d\n", local_boot_state,App.state);
      DPRINT ("AlStatus : 0x%x, AlError : 0x%x, Watchdog : %d \n", (ESCvar.ALstatus & 0x001f),ESCvar.ALerror,wd_cnt);

   }
}

int main (void)
{
   extern void led_run (void *arg);
   extern void led_error (void *arg);
   extern void soes (void *arg);
   extern void my_cyclic_callback (void * arg);

   /* task_spawn ("led_run", led_run, 15, 512, NULL); */
   task_spawn ("led_error", led_error, 15, 512, NULL);
   task_spawn ("t_StatsPrint", my_cyclic_callback, 20, 1024, (void *)NULL);
   task_spawn ("soes", soes, 8, 1024, NULL);

   return (0);
}
