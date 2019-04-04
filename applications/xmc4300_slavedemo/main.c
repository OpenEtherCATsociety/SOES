/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include "ecat_slv.h"
#include "utypes.h"
#include "xmc_gpio.h"

#ifdef XMC4800_F144x2048
#define P_LED  P5_8
#define P_BTN  P15_12
#endif

#ifdef XMC4300_F100x256
#define P_LED  P4_1
#define P_BTN  P3_4
#endif

extern void ESC_eep_handler(void);

/* Application variables */
_Rbuffer    Rb;
_Wbuffer    Wb;
_Cbuffer    Cb;

uint32_t encoder_scale;
uint32_t encoder_scale_mirror;

static const XMC_GPIO_CONFIG_t gpio_config_btn = {
  .mode = XMC_GPIO_MODE_INPUT_INVERTED_PULL_UP,
  .output_level = 0,
  .output_strength = 0
};

static const XMC_GPIO_CONFIG_t gpio_config_led = {
  .mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
  .output_level = XMC_GPIO_OUTPUT_LEVEL_LOW,
  .output_strength = XMC_GPIO_OUTPUT_STRENGTH_STRONG_SOFT_EDGE
};

void cb_get_inputs (void)
{
   Rb.button = XMC_GPIO_GetInput(P_BTN);
   Cb.reset_counter++;
   Rb.encoder =  ESCvar.Time;
}

void cb_set_outputs (void)
{
   if (Wb.LED)
   {
      XMC_GPIO_SetOutputHigh(P_LED);
   }
   else
   {
      XMC_GPIO_SetOutputLow(P_LED);
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
      .user_arg = NULL,
      .use_interrupt = 0,
      .watchdog_cnt = 5000,
      .set_defaults_hook = NULL,
      .pre_state_change_hook = NULL,
      .post_state_change_hook = NULL,
      .application_hook = NULL,
      .safeoutput_override = NULL,
      .pre_object_download_hook = NULL,
      .post_object_download_hook = post_object_download_hook,
      .rxpdo_override = NULL,
      .txpdo_override = NULL,
      .esc_hw_interrupt_enable = NULL,
      .esc_hw_interrupt_disable = NULL,
      .esc_hw_eep_handler = ESC_eep_handler
   };

   DPRINT ("SOES (Simple Open EtherCAT Slave)\n");

   // configure I/O
   XMC_GPIO_Init(P_BTN, &gpio_config_btn);
   XMC_GPIO_Init(P_LED, &gpio_config_led);

   ecat_slv_init (&config);

   while (1)
   {
      ecat_slv();
   }
}

int main (void)
{
   soes (NULL);
   return 0;
}
