#include <stdio.h>
#include "ecat_slv.h"
#include "utypes.h"
#include <bcm2835.h>

/* Application variables */
_Objects    Obj;

#define LED0 RPI_BPLUS_GPIO_J8_40 // GPIO21
#define LED1 RPI_BPLUS_GPIO_J8_38 // GPIO20
#define LED2 RPI_BPLUS_GPIO_J8_36 // GPIO16
#define LED3 RPI_BPLUS_GPIO_J8_32 // GPIO12
#define LED4 RPI_BPLUS_GPIO_J8_18 // GPIO24
#define LED5 RPI_BPLUS_GPIO_J8_16 // GPIO23
#define B0 RPI_BPLUS_GPIO_J8_37 // GPIO26
#define B1 RPI_BPLUS_GPIO_J8_35 // GPIO19
#define B2 RPI_BPLUS_GPIO_J8_33 // GPIO13
#define B3 RPI_BPLUS_GPIO_J8_31 // GPIO06
#define B4 RPI_BPLUS_GPIO_J8_29 // GPIO05
#define B5 RPI_BPLUS_GPIO_J8_15 // GPIO22

void cb_get_inputs (void)
{
   // Assume LEDs connected to 3.3v
   bcm2835_gpio_write(LED0, !Obj.LEDs.LED0);
   bcm2835_gpio_write(LED1, !Obj.LEDs.LED1);
   bcm2835_gpio_write(LED2, !Obj.LEDs.LED2);
   bcm2835_gpio_write(LED3, !Obj.LEDs.LED3);
   bcm2835_gpio_write(LED4, !Obj.LEDs.LED4);
   bcm2835_gpio_write(LED5, !Obj.LEDs.LED5);
}

void cb_set_outputs (void)
{
   // Assume Buttons connected to 3.3v
   Obj.Buttons.Button0 = bcm2835_gpio_lev(B0);
   Obj.Buttons.Button0 = bcm2835_gpio_lev(B1);
   Obj.Buttons.Button0 = bcm2835_gpio_lev(B2);
   Obj.Buttons.Button0 = bcm2835_gpio_lev(B3);
   Obj.Buttons.Button0 = bcm2835_gpio_lev(B4);
   Obj.Buttons.Button0 = bcm2835_gpio_lev(B5);
}

void GPIO_init (void)
{
   bcm2835_gpio_fsel(LED0, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(LED1, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(LED2, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(LED3, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(LED4, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(LED5, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(B0, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(B1, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(B2, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(B3, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(B4, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(B5, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_set_pud(B0, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud(B1, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud(B2, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud(B3, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud(B4, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud(B5, BCM2835_GPIO_PUD_DOWN);
}

int main_run (void * arg)
{
   static esc_cfg_t config =
   {
      .user_arg = "rpi3,cs0",
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

   printf ("Hello Main\n");
   ecat_slv_init (&config);

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
