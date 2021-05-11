#include <stdio.h>
#include <string.h>
#include "ecat_options.h"
#include "ecat_slv.h"
#include "esc_hw.h"
#include "utypes.h"

/* Application variables */
_Objects    Obj;

#if !(DYN_PDO_MAPPING)
uint8_t * txpdo = Obj.txpdo;
uint8_t * rxpdo = Obj.rxpdo;
#endif


void cb_state_change (uint8_t * as, uint8_t * an)
{
   if (*as == SAFEOP_TO_OP)
   {
      /* Enable HW watchdog event */
      ESC_ALeventmaskwrite (ESC_ALeventmaskread() | ESCREG_ALEVENT_WD);
      /* Enable SM2 sync manager event */
      ESC_ALeventmaskwrite (ESC_ALeventmaskread() | ESCREG_ALEVENT_SM2);
   }
}

void cb_application (void)
{
   /* Check for watchdog expired event */
   if (ESCvar.ALevent & ESCREG_ALEVENT_WD)
   {
      if ((ESC_WDstatus() & 0x01) &&
          ((CC_ATOMIC_GET(ESCvar.App.state) & APPSTATE_OUTPUT) > 0))
      {
         DPRINT("DIG_process watchdog expired\n");
         ESC_ALstatusgotoerror((ESCsafeop | ESCerror), ALERR_WATCHDOG);
      }
   }
}

void cb_get_inputs (void)
{
   /* Loop back outputs to inputs */
   #if DYN_PDO_MAPPING
   memcpy(Obj.txpdo,Obj.rxpdo,BYTE_NUM);
   #else
   memcpy(txpdo,rxpdo,BYTE_NUM);
   #endif
}

void cb_set_outputs (void)
{
}

#if !(DYN_PDO_MAPPING)
/** Write local process data to Sync Manager 3, Master Inputs.
 */
void cb_txpdo_update (void)
{
   ESC_write (ESC_SM3_sma, txpdo, BYTE_NUM);
}

/** Read Sync Manager 2 to local process data, Master Outputs.
 */
void cb_rxpdo_update (void)
{
   ESC_read (ESC_SM2_sma, rxpdo, BYTE_NUM);
}
#endif

void cb_safeoutput (void)
{
   /* Set all outputs to zero */
   #if DYN_PDO_MAPPING
   memset(Obj.rxpdo,0,BYTE_NUM);
   #else
   memset(rxpdo,0,BYTE_NUM);
   #endif
}

int soes (void * arg)
{
   static esc_cfg_t config =
   {
      .user_arg = "rpi3,cs0", /* Change rpi3 to rpi4 for raspberry pi 4 or later */
      .use_interrupt = 0,
      .watchdog_cnt = INT32_MAX, /* Use HW SM watchdog instead */
      .set_defaults_hook = NULL,
      .pre_state_change_hook = NULL,
      .post_state_change_hook = cb_state_change,
      .application_hook = cb_application,
      .safeoutput_override = cb_safeoutput,
      .pre_object_download_hook = NULL,
      .post_object_download_hook = NULL,
      #if DYN_PDO_MAPPING
      .rxpdo_override = NULL,
      .txpdo_override = NULL,
      #else
      .rxpdo_override = cb_rxpdo_update,
      .txpdo_override = cb_txpdo_update,
      #endif
      .esc_hw_interrupt_enable = NULL,
      .esc_hw_interrupt_disable = NULL,
      .esc_hw_eep_handler = NULL,
      .esc_check_dc_handler = NULL,
   };

   DPRINT ("Start slave init\n");
   ecat_slv_init (&config);
   DPRINT ("Initialization finished\n");
   
   while (1)
   {
      ecat_slv();
   }

   return 0;
}

int main (void)
{
   DPRINT ("SOES (Simple Open EtherCAT Slave)\n");
   soes (NULL);
   return 0;
}
