#include <stddef.h>
#include "utypes.h"
#include "esc.h"
#include "esc_coe.h"
#include "esc_foe.h"
#include "config.h"
#include "slave.h"

volatile _ESCvar  ESCvar;
/* Make room for largest MBX */
uint8_t           MBX[MBXBUFFERS * MAX(MBXSIZE,MBXSIZEBOOT)];
_MBXcontrol       MBXcontrol[MBXBUFFERS];
uint8_t           MBXrun=0;
uint16_t          ESC_SM2_sml, ESC_SM3_sml;
_Rbuffer          Rb;
_Wbuffer          Wb;
_Cbuffer          Cb;
_Mbuffer          Mb;
_App              App;
uint16_t          TXPDOsize,RXPDOsize;
uint8_t           dc_sync = 0;
int8_t            sync_counter = 0;
uint16_t          sync_counter_limit = 0;

static int watchdog;

/** Mandatory: Function to pre-qualify the incoming SDO download.
 *
 * @param[in] index      = index of SDO download request to check
 * @param[in] sub-index  = sub-index of SDO download request to check
 * @return 1 if the SDO Download is correct. 0 If not correct.
 */
int ESC_pre_objecthandler (uint16_t index, uint8_t subindex)
{
   int result = 1;

   if(esc_cfg->pre_object_download_hook)
   {
      result = (esc_cfg->pre_object_download_hook)(index, subindex);
   }

   return result;
}

/** Mandatory: Hook called from the slave stack SDO Download handler to act on
 * user specified Index and Sub-index.
 *
 * @param[in] index      = index of SDO download request to handle
 * @param[in] sub-index  = sub-index of SDO download request to handle
 */
void ESC_objecthandler (uint16_t index, uint8_t subindex)
{
   switch (index)
   {
   /* Handle post-write of parameter values */
   case 0x8001:
   {
      cb_post_write_variableRW(subindex);
      break;
   }
   default:
   {
      if(esc_cfg->post_object_download_hook != NULL)
      {
         (esc_cfg->post_object_download_hook)(index, subindex);
      }
      break;
   }
   }
}

/** Mandatory: Hook called from the slave stack ESC_stopoutputs to act on state changes
 * forcing us to stop outputs. Here we can set them to a safe state.
 * set
 */
void APP_safeoutput (void)
{
   DPRINT ("APP_safeoutput\n");

   if(esc_cfg->safeoutput_override != NULL)
   {
      (esc_cfg->safeoutput_override)();
   }
   else
   {
      // Set safe values for Wb.LEDgroup0
      Wb.LEDgroup0.LED0 = 0;
      // Set safe values for Wb.LEDgroup1
      Wb.LEDgroup1.LED1 = 0;
   }
}

/** Mandatory: Write local process data to Sync Manager 3, Master Inputs.
 */
void TXPDO_update (void)
{
   if(esc_cfg->txpdo_override != NULL)
   {
      (esc_cfg->txpdo_override )();
   }
   else
   {
      ESC_write (SM3_sma, &Rb, TXPDOsize);
   }
}

/** Mandatory: Read Sync Manager 2 to local process data, Master Outputs.
 */
void RXPDO_update (void)
{
   if(esc_cfg->rxpdo_override != NULL)
   {
      (esc_cfg->rxpdo_override )();
   }
   else
   {
      ESC_read (SM2_sma, &Wb, RXPDOsize);
   }
}

/** Mandatory: Function to update local I/O, call read ethercat outputs, call
 * write ethercat inputs. Implement watch-dog counter to count-out if we have
 * made state change affecting the App.state.
 */
void DIG_process (uint8_t flags)
{
   /* Handle watchdog */
   if((flags & DIG_PROCESS_WD_FLAG) > 0)
   {

      if (CC_ATOMIC_GET(watchdog) > 0)
      {
         CC_ATOMIC_SUB(watchdog, 1);
      }

      if ((CC_ATOMIC_GET(watchdog) <= 0) &&
          ((CC_ATOMIC_GET(App.state) & APPSTATE_OUTPUT) > 0))
      {
         DPRINT("DIG_process watchdog expired\n");
         ESC_stopoutput();
         /* watchdog, invalid outputs */
         ESC_ALerror (ALERR_WATCHDOG);
         /* goto safe-op with error bit set */
         ESC_ALstatus (ESCsafeop | ESCerror);
      }
      else if(((CC_ATOMIC_GET(App.state) & APPSTATE_OUTPUT) == 0))
      {
         CC_ATOMIC_SET(watchdog, esc_cfg->watchdog_cnt);
      }
   }

   /* Handle Outputs */
   if ((flags & DIG_PROCESS_OUTPUTS_FLAG) > 0)
   {
      if(((CC_ATOMIC_GET(App.state) & APPSTATE_OUTPUT) > 0) &&
         (ESCvar.ALevent & ESCREG_ALEVENT_SM2))
      {
         RXPDO_update();
         CC_ATOMIC_SET(watchdog, esc_cfg->watchdog_cnt);
         if(dc_sync > 0)
         {
            CC_ATOMIC_ADD(sync_counter,1);
         }
         /* Set outputs */
         cb_set_LEDgroup0();
         cb_set_LEDgroup1();
      }
      else if (ESCvar.ALevent & ESCREG_ALEVENT_SM2)
      {
         RXPDO_update();
      }
   }

   /* Call application */
   if ((flags & DIG_PROCESS_APP_HOOK_FLAG) > 0)
   {

      if((CC_ATOMIC_GET(App.state) & APPSTATE_OUTPUT) > 0)
      {
         CC_ATOMIC_SUB(sync_counter,1);
      }

      if((dc_sync > 0) &&
            ((CC_ATOMIC_GET(sync_counter) < -sync_counter_limit) ||
             (CC_ATOMIC_GET(sync_counter) > sync_counter_limit)))
      {
         if((CC_ATOMIC_GET(App.state) & APPSTATE_OUTPUT) > 0)
         {
            DPRINT("sync error = %d\n", sync_counter);
            ESC_stopoutput();
            /* Sync error */
            ESC_ALerror (ALERR_SYNCERROR);
            /* goto safe-op with error bit set */
            ESC_ALstatus (ESCsafeop | ESCerror);
            CC_ATOMIC_SET(sync_counter,0);
         }
      }
      /* Call application callback if set */
      if (esc_cfg->application_hook != NULL)
      {
         (esc_cfg->application_hook)();
      }
   }

   /* Handle Inputs */
   if ((flags & DIG_PROCESS_INPUTS_FLAG) > 0)
   {
      if(CC_ATOMIC_GET(App.state) > 0)
      {
         /* Update inputs */
         cb_get_Buttons();
         TXPDO_update();
      }
   }
}

/**
 * ISR function. It should be called from ISR for applications entirely driven by
 * interrupts.
 * Read and handle events for the EtherCAT state, status, mailbox and eeprom.
 */
void ecat_slv_isr (void)
{
   do
   {
      /* Check the state machine */
      ESC_state();
      /* Check the SM activation event */
      ESC_sm_act_event();

      /* Check mailboxes */
      while ((ESC_mbxprocess() > 0) || (ESCvar.txcue > 0))
      {
         ESC_coeprocess();
         ESC_foeprocess();
         ESC_xoeprocess();
      }

      /* Call emulated eeprom handler if set */
      if (esc_cfg->esc_hw_eep_handler != NULL)
      {
         (esc_cfg->esc_hw_eep_handler)();
      }

      CC_ATOMIC_SET(ESCvar.ALevent,ESC_ALeventread());

   }while(ESCvar.ALevent & (ESCREG_ALEVENT_CONTROL | ESCREG_ALEVENT_SMCHANGE
         | ESCREG_ALEVENT_SM0 | ESCREG_ALEVENT_SM1 | ESCREG_ALEVENT_EEP));

   ESC_ALeventmaskwrite(ESC_ALeventmaskread()
         | (ESCREG_ALEVENT_CONTROL | ESCREG_ALEVENT_SMCHANGE
         | ESCREG_ALEVENT_SM0 | ESCREG_ALEVENT_SM1
         | ESCREG_ALEVENT_EEP));
}

/**
 * Polling function. It should be called periodically for an application 
 * when only SM2/DC interrupt is active.
 * Read and handle events for the EtherCAT state, status, mailbox and eeprom.
 */
void ecat_slv_poll (void)
{
   /* Read local time from ESC*/
   ESC_read (ESCREG_LOCALTIME, (void *) &ESCvar.Time, sizeof (ESCvar.Time));
   ESCvar.Time = etohl (ESCvar.Time);

   /* Check the state machine */
   ESC_state();
   /* Check the SM activation event */
   ESC_sm_act_event();

   /* Check mailboxes */
   if (ESC_mbxprocess())
   {
      ESC_coeprocess();
      ESC_foeprocess();
      ESC_xoeprocess();
   }

   /* Call emulated eeprom handler if set */
   if (esc_cfg->esc_hw_eep_handler != NULL)
   {
      (esc_cfg->esc_hw_eep_handler)();
   }
}

void ecat_slv (void)
{
   ecat_slv_poll();
   DIG_process(DIG_PROCESS_WD_FLAG | DIG_PROCESS_OUTPUTS_FLAG |
         DIG_PROCESS_APP_HOOK_FLAG | DIG_PROCESS_INPUTS_FLAG);
}

/**
 * Initialize the slave stack.
 */
void ecat_slv_init (esc_cfg_t * config)
{
   DPRINT ("Slave stack init started\n");

   TXPDOsize = ESC_SM3_sml = sizeOfPDO(TX_PDO_OBJIDX);
   RXPDOsize = ESC_SM2_sml = sizeOfPDO(RX_PDO_OBJIDX);

   /* Init watchdog */
   watchdog = config->watchdog_cnt;

   /* Call stack configuration */
   ESC_config (config);
   /* Call HW init */
   ESC_init (config);

   /*  wait until ESC is started up */
   while ((ESCvar.DLstatus & 0x0001) == 0)
   {
      ESC_read (ESCREG_DLSTATUS, (void *) &ESCvar.DLstatus,
                sizeof (ESCvar.DLstatus));
      ESCvar.DLstatus = etohs (ESCvar.DLstatus);
   }

   /* Init FoE */
   FOE_init();

   /* reset ESC to init state */
   ESC_ALstatus (ESCinit);
   ESC_ALerror (ALERR_NONE);
   ESC_stopmbx();
   ESC_stopinput();
   ESC_stopoutput();
}
