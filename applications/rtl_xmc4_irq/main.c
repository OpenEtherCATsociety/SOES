#include <kern.h>
#include <xmc4.h>
#include <bsp.h>
#include "slave_irq.h"
#include "soes/esc_hw.h"
#include "soes/esc.h"


/**
 * This function reads physical input values and assigns the corresponding members
 * of Rb.Buttons
 */
void cb_get_Buttons()
{
 Rb.Buttons.Button1 = gpio_get(GPIO_BUTTON1);
}

/**
 * This function writes physical output values from the corresponding members of
 * Wb.LEDs
 */
void cb_set_LEDgroup0()
{
   gpio_set(GPIO_LED1, Wb.LEDgroup0.LED0);
}

/**
 * This function writes physical output values from the corresponding members of
 * Wb.LEDs
 */
void cb_set_LEDgroup1()
{
   gpio_set(GPIO_LED2, Wb.LEDgroup1.LED1);
}

/**
 * This function is called after a SDO write of the object Cb.Parameters.
 */
void cb_post_write_Parameters(int subindex)
{

}

void isr_run(void * arg)
{
   while(1)
   {
      sem_wait(ecat_isr_sem);
      soes_isr();
   }
}

/** Optional: Hook called after state change for application specific
 * actions for specific state changes.
 */
void post_state_change_hook (uint8_t * as, uint8_t * an)
{
#if 0
   /* Add specific step change hooks here */
   if ((*as == BOOT_TO_INIT) && (*an == ESCinit))
   {
      boot_inithook();
   }
   else if((*as == INIT_TO_BOOT) && (*an & ESCerror ) == 0)
   {
      init_boothook();
   }
#endif
}

/* Configuration parameters for SOES */
static esc_cfg_t config =
{
   .watchdog_cnt = 100, /* Non UNIT watchdog counter, for the application developer to decide UNIT.
                         * This example set 100 cnt and by call soes every 1ms, it create an watchdog
                         * running at ~100ms.
                         */
   .pre_state_change_hook = NULL, /* Hook called before state transition */
   .post_state_change_hook = post_state_change_hook, /* Hook called after state transition */
   .application_loop_callback = NULL /* Callback in application loop */
};

void main_run(void * arg)
{
   soes_init_irq(&config);

   ecat_isr_sem = sem_create(0);
   task_spawn ("soes_isr", isr_run, 9, 2048, NULL);
   int_connect (IRQ_ECAT0_SR0, ecat_isr, NULL);
   int_enable (IRQ_ECAT0_SR0);

   while(1)
   {
      soes_irq();
      task_delay(1);
   }
}

int main(void)
{
   rprintf("Hello Main\n");
   task_spawn ("soes", main_run, 8, 2048, NULL);

   return 0;
}

