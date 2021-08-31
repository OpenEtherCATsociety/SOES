#include <stdio.h>
#include "ecat_slv.h"
#include "utypes.h"
#include <bcm2835.h>

/* Application variables */
_Objects    Obj;

#define GPIO21 RPI_BPLUS_GPIO_J8_40
#define GPIO20 RPI_BPLUS_GPIO_J8_38
#define GPIO16 RPI_BPLUS_GPIO_J8_36
#define GPIO12 RPI_BPLUS_GPIO_J8_32
#define GPIO24 RPI_BPLUS_GPIO_J8_18
#define GPIO23 RPI_BPLUS_GPIO_J8_16
#define GPIO26 RPI_BPLUS_GPIO_J8_37
#define GPIO19 RPI_BPLUS_GPIO_J8_35
#define GPIO13 RPI_BPLUS_GPIO_J8_33
#define GPIO06 RPI_BPLUS_GPIO_J8_31
#define GPIO05 RPI_BPLUS_GPIO_J8_29
#define GPIO22 RPI_BPLUS_GPIO_J8_15

void cb_get_inputs (void)
{
   // Assume LEDs connected to 3.3v
   bcm2835_gpio_write(GPIO21, (Obj.LEDs.LED0 ? LOW : HIGH));
   bcm2835_gpio_write(GPIO20, (Obj.LEDs.LED1 ? LOW : HIGH));
   bcm2835_gpio_write(GPIO16, (Obj.LEDs.LED2 ? LOW : HIGH));
   bcm2835_gpio_write(GPIO12, (Obj.LEDs.LED3 ? LOW : HIGH));
   bcm2835_gpio_write(GPIO24, (Obj.LEDs.LED4 ? LOW : HIGH));
   bcm2835_gpio_write(GPIO23, (Obj.LEDs.LED5 ? LOW : HIGH));
}

void cb_set_outputs (void)
{
   // Assume Buttons connected to 3.3v
   Obj.Buttons.Button0 = bcm2835_gpio_lev(GPIO26);
   Obj.Buttons.Button0 = bcm2835_gpio_lev(GPIO19);
   Obj.Buttons.Button0 = bcm2835_gpio_lev(GPIO13);
   Obj.Buttons.Button0 = bcm2835_gpio_lev(GPIO06);
   Obj.Buttons.Button0 = bcm2835_gpio_lev(GPIO05);
   Obj.Buttons.Button0 = bcm2835_gpio_lev(GPIO22);
}

void GPIO_init (void)
{
   bcm2835_init();
   // Assume LEDs connected to 3.3v side of header
   bcm2835_gpio_fsel(GPIO21, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(GPIO20, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(GPIO16, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(GPIO12, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(GPIO24, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(GPIO23, BCM2835_GPIO_FSEL_OUTP);
   // Assume buttons connected to 5v side of header
   // Do not bridge to 5v, the ports might burn
   bcm2835_gpio_fsel(GPIO26, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(GPIO19, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(GPIO13, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(GPIO06, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(GPIO05, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(GPIO22, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_set_pud(GPIO26, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud(GPIO19, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud(GPIO13, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud(GPIO06, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud(GPIO05, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud(GPIO22, BCM2835_GPIO_PUD_DOWN);
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
   GPIO_init();
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
