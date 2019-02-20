
#ifndef SOES_V1
#include <stddef.h>
#include "esc.h"
#include "esc_coe.h"
#include "esc_foe.h"
#include "ecat_slv.h"

#define IS_RXPDO(index) ((index) >= 0x1600 && (index) < 0x1800)
#define IS_TXPDO(index) ((index) >= 0x1A00 && (index) < 0x1C00)

/* Global variables used by the stack */
extern _ESCvar ESCvar;
extern _SMmap  SMmap2[];
extern _SMmap  SMmap3[];

/* Private variables */
static volatile int watchdog;

/** Mandatory: Function to pre-qualify the incoming SDO download.
 *
 * @param[in] index      = index of SDO download request to check
 * @param[in] sub-index  = sub-index of SDO download request to check
 * @return SDO abort code, or 0 on success
 */
uint32_t ESC_pre_objecthandler (uint16_t index,
      uint8_t subindex,
      void * data,
      size_t size,
      bool isCA)
{
   int abort = 0;

   if (IS_RXPDO (index) ||
       IS_TXPDO (index) ||
       index == RX_PDO_OBJIDX ||
       index == TX_PDO_OBJIDX)
   {
      if (subindex > 0 && COE_maxSub (index) != 0)
      {
         abort = ABORT_SUBINDEX0_NOT_ZERO;
      }
   }

   if (ESCvar.pre_object_download_hook)
   {
      abort = (ESCvar.pre_object_download_hook) (index,
            subindex,
            data,
            size,
            isCA);
   }

   return abort;
}

/** Mandatory: Hook called from the slave stack SDO Download handler to act on
 * user specified Index and Sub-index.
 *
 * @param[in] index      = index of SDO download request to handle
 * @param[in] sub-index  = sub-index of SDO download request to handle
 */
void ESC_objecthandler (uint16_t index, uint8_t subindex, bool isCA)
{
   if (ESCvar.post_object_download_hook != NULL)
   {
      (ESCvar.post_object_download_hook)(index, subindex, isCA);
   }
}

/** Mandatory: Hook called from the slave stack ESC_stopoutputs to act on state changes
 * forcing us to stop outputs. Here we can set them to a safe state.
 * set
 */
void APP_safeoutput (void)
{
   DPRINT ("APP_safeoutput\n");

   if(ESCvar.safeoutput_override != NULL)
   {
      (ESCvar.safeoutput_override)();
   }
}

/** Mandatory: Write local process data to Sync Manager 3, Master Inputs.
 */
void TXPDO_update (void)
{
   if(ESCvar.txpdo_override != NULL)
   {
      (ESCvar.txpdo_override)();
   }
   else
   {
      COE_pdoPack (ESCvar.txpdos_address, ESCvar.sm3mappings, SMmap3);
      ESC_write (ESC_SM3_sma, ESCvar.txpdos_address , ESCvar.ESC_SM3_sml);
   }
}

/** Mandatory: Read Sync Manager 2 to local process data, Master Outputs.
 */
void RXPDO_update (void)
{
   if(ESCvar.rxpdo_override != NULL)
   {
      (ESCvar.rxpdo_override)();
   }
   else
   {
      ESC_read (ESC_SM2_sma, ESCvar.rxpdos_address, ESCvar.ESC_SM2_sml);
      COE_pdoUnpack (ESCvar.rxpdos_address, ESCvar.sm2mappings, SMmap2);
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
          ((CC_ATOMIC_GET(ESCvar.App.state) & APPSTATE_OUTPUT) > 0))
      {
         DPRINT("DIG_process watchdog expired\n");
         ESC_ALstatusgotoerror((ESCsafeop | ESCerror), ALERR_WATCHDOG);
      }
      else if(((CC_ATOMIC_GET(ESCvar.App.state) & APPSTATE_OUTPUT) == 0))
      {
         CC_ATOMIC_SET(watchdog, ESCvar.watchdogcnt);
      }
   }

   /* Handle Outputs */
   if ((flags & DIG_PROCESS_OUTPUTS_FLAG) > 0)
   {
      if(((CC_ATOMIC_GET(ESCvar.App.state) & APPSTATE_OUTPUT) > 0) &&
         (ESCvar.ALevent & ESCREG_ALEVENT_SM2))
      {
         RXPDO_update();
         CC_ATOMIC_SET(watchdog, ESCvar.watchdogcnt);
         /* Set outputs */
         cb_set_outputs();
      }
      else if (ESCvar.ALevent & ESCREG_ALEVENT_SM2)
      {
         RXPDO_update();
      }
   }

   /* Call application */
   if ((flags & DIG_PROCESS_APP_HOOK_FLAG) > 0)
   {
      /* Call application callback if set */
      if (ESCvar.application_hook != NULL)
      {
         (ESCvar.application_hook)();
      }
   }

   /* Handle Inputs */
   if ((flags & DIG_PROCESS_INPUTS_FLAG) > 0)
   {
      if(CC_ATOMIC_GET(ESCvar.App.state) > 0)
      {
         /* Update inputs */
         cb_get_inputs();
         TXPDO_update();
      }
   }
}

/**
 * Handler for SM change, SM0/1, AL CONTROL and EEPROM events, the application
 * control what interrupts that should be served and re-activated with
 * event mask argument
 */
void ecat_slv_worker (uint32_t event_mask)
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
#if USE_FOE
         ESC_foeprocess();
#endif
         ESC_xoeprocess();
      }

      /* Call emulated eeprom handler if set */
      if (ESCvar.esc_hw_eep_handler != NULL)
      {
         (ESCvar.esc_hw_eep_handler)();
      }

      CC_ATOMIC_SET(ESCvar.ALevent, ESC_ALeventread());

   }while(ESCvar.ALevent & event_mask);

   ESC_ALeventmaskwrite(ESC_ALeventmaskread() | event_mask);
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
#if USE_FOE
      ESC_foeprocess();
#endif
      ESC_xoeprocess();
   }

   /* Call emulated eeprom handler if set */
   if (ESCvar.esc_hw_eep_handler != NULL)
   {
      (ESCvar.esc_hw_eep_handler)();
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

#if USE_FOE
   /* Init FoE */
   FOE_init();
#endif

   /* reset ESC to init state */
   ESC_ALstatus (ESCinit);
   ESC_ALerror (ALERR_NONE);
   ESC_stopmbx();
   ESC_stopinput();
   ESC_stopoutput();
}
#endif
