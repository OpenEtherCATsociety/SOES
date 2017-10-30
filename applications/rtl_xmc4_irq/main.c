#include <kern.h>
#include <xmc4.h>
#include <bsp.h>
#include "slave.h"
#include "esc_hw.h"
#include "config.h"

#define SAMPLE_CODE 0

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
void cb_post_write_variableRW(int subindex)
{

}

/** Optional: Hook called after state change for application specific
 * actions for specific state changes.
 */
void pre_state_change_hook (uint8_t * as, uint8_t * an)
{
}

/** Optional: Hook called after state change for application specific
 * actions for specific state changes.
 */
void post_state_change_hook (uint8_t * as, uint8_t * an)
{
#if SAMPLE_CODE
   /* Add specific step change hooks here */
   if ((*as == BOOT_TO_INIT) && (*an == ESCinit))
   {
      rprintf("boot BOOT_TO_INIT\n");
      upgrade_finished();
      /* If we return here */
      ESC_ALerror (ALERR_NOVALIDFIRMWARE);
      /* Upgrade failed, enter init with error */
      *an  = (ESCinit | ESCerror);
   }
   else if((*as == PREOP_TO_SAFEOP))
   {
      rprintf("boot PREOP_TO_SAFEOP\n");
      ESC_ALerror (ALERR_NOVALIDFIRMWARE);
      /* Stay in preop with error bit set */
      *an = (ESCpreop | ESCerror);
   }
#endif
}

void user_post_dl_objecthandler (uint16_t index, uint8_t subindex)
{
#if SAMPLE_CODE
   switch (index)
   {
   case 0x1c12:
   {
      RXPDOsize = ESC_SM2_sml = sizeRXPDO();
      break;
   }

   /* Handle post-write of parameter values */
   default:
      break;
   }
#endif
}

int user_pre_dl_objecthandler (uint16_t index, uint8_t subindex)
{
#if SAMPLE_CODE
   if (index == 0x1c12)
   {
      SDO_abort (index, subindex, ABORT_READONLY);
      return 0;
   }
   if (index == 0x1c13)
   {
      SDO_abort (index, subindex, ABORT_READONLY);
      return 0;
   }
#endif
   return 1;
}


/* Called from stack when stopping outputs */
void user_safeoutput (void)
{
#if SAMPLE_CODE
   DPRINT ("APP_safeoutput\n");

   // Set safe values for Wb.LEDgroup0
   Wb.LEDgroup0.LED0 = 1;

   // Set safe values for Wb.LEDgroup1
   Wb.LEDgroup1.LED1 = 1;
#endif
}

/* Configuration parameters for SOES
 * SM and Mailbox parameters comes from the
 * generated config.h
 */
static esc_cfg_t config =
{
   .user_arg = NULL,
   .use_interrupt = 1,
   .watchdog_cnt = 100,
   .mbxsize = MBXSIZE,
   .mbxsizeboot = MBXSIZEBOOT,
   .mbxbuffers = MBXBUFFERS,
   .mb[0] = {MBX0_sma, MBX0_sml, MBX0_sme, MBX0_smc, 0},
   .mb[1] = {MBX1_sma, MBX1_sml, MBX1_sme, MBX1_smc, 0},
   .mb_boot[0] = {MBX0_sma_b, MBX0_sml_b, MBX0_sme_b, MBX0_smc_b, 0},
   .mb_boot[1] = {MBX1_sma_b, MBX1_sml_b, MBX1_sme_b, MBX1_smc_b, 0},
   .pdosm[0] = {SM2_sma, 0, 0, SM2_smc, SM2_act},
   .pdosm[1] = {SM3_sma, 0, 0, SM3_smc, SM3_act},
   .pre_state_change_hook = NULL,
   .post_state_change_hook = NULL,
   .application_hook = NULL,
   .safeoutput_override = user_safeoutput,
   .pre_object_download_hook = user_pre_dl_objecthandler,
   .post_object_download_hook = user_post_dl_objecthandler,
   .rxpdo_override = NULL,
   .txpdo_override = NULL,
   .esc_hw_interrupt_enable = ESC_interrupt_enable,
   .esc_hw_interrupt_disable = ESC_interrupt_disable,
   .esc_hw_eep_handler = ESC_eep_handler
};

void main_run(void * arg)
{
   ecat_slv_init(&config);

   while(1)
   {
      if(config.use_interrupt != 0)
      {
         DIG_process(DIG_PROCESS_WD_FLAG);
      }
      else
      {
         ecat_slv();
      }
      task_delay(1);
   }
}

int main(void)
{
   rprintf("Hello Main\n");
   task_spawn ("soes", main_run, 8, 2048, NULL);

   return 0;
}

