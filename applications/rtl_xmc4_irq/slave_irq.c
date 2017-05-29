#include <stddef.h>
#include "utypes.h"
#include "soes/esc.h"
#include "soes/esc_coe.h"
#include "soes/esc_foe.h"
#include "soes/esc_hw.h"
#include "slave_irq.h"

#if EEP_EMULATION
#include "soes/esc_eep.h"
#include "soes/esc_hw_eep.h"
#endif

#define DEFAULTTXPDOMAP    0x1a00
#define DEFAULTRXPDOMAP    0x1600
#define DEFAULTTXPDOITEMS  1
#define DEFAULTRXPDOITEMS  1
#define SYNC_ERROR_LIMIT   2

volatile _ESCvar  ESCvar;
_MBX              MBX[MBXBUFFERS];
_MBXcontrol       MBXcontrol[MBXBUFFERS];
uint8_t           MBXrun=0;
uint16_t          SM2_sml,SM3_sml;
_Rbuffer          Rb;
_Wbuffer          Wb;
_Cbuffer          Cb;
_Mbuffer          Mb;
_App              App;
uint16_t          TXPDOsize,RXPDOsize;
uint16_t          txpdomap = DEFAULTTXPDOMAP;
uint16_t          rxpdomap = DEFAULTRXPDOMAP;
uint8_t           txpdoitems = DEFAULTTXPDOITEMS;
uint8_t           rxpdoitems = DEFAULTTXPDOITEMS;
uint8_t           dc_sync;
int8_t            sync_counter;


static unsigned int watchdog;
static unsigned int watchdog_reset;
static void (*application_loop_callback)(void) = NULL;

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
   case 0x1c12:
   {
      if (rxpdoitems > 1)
      {
         rxpdoitems = 1;
      }
      if ((rxpdomap != 0x1600) && (rxpdomap != 0x1601)
          && (rxpdomap != 0x0000))
      {
         rxpdomap = 0x1600;
      }
      RXPDOsize = SM2_sml = sizeRXPDO();
      break;
   }
   case 0x1c13:
   {
      if (txpdoitems > 1)
      {
         txpdoitems = 1;
      }
      if ((txpdomap != 0x1A00) && (txpdomap != 0x1A01)
          && (rxpdomap != 0x0000))
      {
         txpdomap = 0x1A00;
      }
      TXPDOsize = SM3_sml = sizeTXPDO();
      break;
   }
   /* Handle post-write of parameter values */
   case 0x8000:
   {
      cb_post_write_Parameters(subindex);
      break;
   }

   /* Handle SYNC of parameter values */
   case 0x1C32:
   {

      break;
   }
   default:
      break;
   }
}

/** Mandatory: Hook called from the slave stack ESC_stopoutputs to act on state changes
 * forcing us to stop outputs. Here we can set them to a safe state.
 * set
 */
void APP_safeoutput (void)
{
   DPRINT ("APP_safeoutput\n");

   // Set safe values for Wb.LEDgroup0
   Wb.LEDgroup0.LED0 = 0;

   // Set safe values for Wb.LEDgroup1
   Wb.LEDgroup1.LED1 = 0;

}

/** Mandatory: Write local process data to Sync Manager 3, Master Inputs.
 */
void TXPDO_update (void)
{
   ESC_write (SM3_sma, &Rb, TXPDOsize);
}
/** Mandatory: Read Sync Manager 2 to local process data, Master Outputs.
 */
void RXPDO_update (void)
{
   ESC_read (SM2_sma, &Wb, RXPDOsize);
}

/** Mandatory: Function to update local I/O, call read ethercat outputs, call
 * write ethercat inputs. Implement watch-dog counter to count-out if we have
 * made state change affecting the App.state.
 */
void DIG_process (uint8_t flags)
{
   /* Handle watchdog */
   if((flags & DIG_PROCESS_WD) > 0)
   {

      if (ATOMIC_GET(watchdog) > 0)
      {
         ATOMIC_SUB(watchdog, 1);
      }

      if ((ATOMIC_GET(watchdog) == 0) &&
          ((ATOMIC_GET(App.state) & APPSTATE_OUTPUT) > 0))
      {
         DPRINT("DIG_process watchdog expired\n");
         ESC_stopoutput();
         /* watchdog, invalid outputs */
         ESC_ALerror (ALERR_WATCHDOG);
         /* goto safe-op with error bit set */
         ESC_ALstatus (ESCsafeop | ESCerror);
      }
      else if(((ATOMIC_GET(App.state) & APPSTATE_OUTPUT) == 0))
      {
         ATOMIC_SET(watchdog, watchdog_reset);
      }
   }

   /* Handle Outputs */
   if ((flags & DIG_PROCESS_OUTPUTS) > 0)
   {
      if((ATOMIC_GET(App.state) & APPSTATE_OUTPUT) > 0)
      {
         RXPDO_update();

         ATOMIC_SET(watchdog, watchdog_reset);

         /* Set outputs */
         cb_set_LEDgroup0();
         cb_set_LEDgroup1();
         if(dc_sync > 0)
         {
            ATOMIC_ADD(sync_counter,1);
         }
      }
      else
      {
         /* SM2 trigger ? */
         if (ESCvar.ALevent & ESCREG_ALEVENT_SM2)
         {
            RXPDO_update();
         }
      }
   }

   /* Call appliction */
   if ((flags & DIG_PROCESS_APP_HOOK) > 0)
   {

      if((ATOMIC_GET(App.state) & APPSTATE_OUTPUT) > 0)
      {
         ATOMIC_SUB(sync_counter,1);
      }

      if((dc_sync > 0) &&
            ((ATOMIC_GET(sync_counter) < -SYNC_ERROR_LIMIT) ||
             (ATOMIC_GET(sync_counter) > SYNC_ERROR_LIMIT)))
      {
         if((ATOMIC_GET(App.state) & APPSTATE_OUTPUT) > 0)
         {
            rprintp("sync error = %d\n",sync_counter);
            ESC_stopoutput();
            /* Sync error */
            ESC_ALerror (ALERR_SYNCERROR);
            /* goto safe-op with error bit set */
            ESC_ALstatus (ESCsafeop | ESCerror);
            ATOMIC_SET(sync_counter,0);
         }
      }
      /* Call application callback if set */
      if (application_loop_callback != NULL)
      {
         (application_loop_callback)();
      }
   }

   /* Handle Inputs */
   if ((flags & DIG_PROCESS_INPUTS) > 0)
   {
      if(ATOMIC_GET(App.state) > 0)
      {
         /* Update inputs */
         cb_get_Buttons();
         TXPDO_update();
      }
   }
}

/********** TODO: Generic code beyond this point ***************/

/**
 * SOES IRQ function. It should be called by the ISR handling task or similar.
 * Reads the EtherCAT state and status, handles mailboxchanges nad EEPROM
 * requests.
 */
void soes_isr (void)
{

   do
   {
      /* Check the state machine */
      ESC_state();
      /* Check the SM activation event */
      ESC_sm_act_event();

      /* Check mailboxes */
      /* Check mailboxes */
      while ((ESC_mbxprocess() > 0) || (ESCvar.txcue > 0))
      {
         ESC_coeprocess();
         ESC_foeprocess();
         ESC_xoeprocess();
      }

#if EEP_EMULATION
      EEP_process ();
      EEP_hw_process();
#endif
      ATOMIC_SET(ESCvar.ALevent,ESC_ALeventread());

   }while(ESCvar.ALevent & (ESCREG_ALEVENT_CONTROL | ESCREG_ALEVENT_SMCHANGE
         | ESCREG_ALEVENT_SM0 | ESCREG_ALEVENT_SM1 | ESCREG_ALEVENT_EEP));

   ESC_ALeventmaskwrite(ESC_ALeventmaskread()
         | (ESCREG_ALEVENT_CONTROL | ESCREG_ALEVENT_SMCHANGE
         | ESCREG_ALEVENT_SM0 | ESCREG_ALEVENT_SM1
         | ESCREG_ALEVENT_EEP));

   /* On init restore PDO mappings to default size */
   if((ESCvar.ALstatus & 0x0f) == ESCinit)
   {
      txpdomap = DEFAULTTXPDOMAP;
      rxpdomap = DEFAULTRXPDOMAP;
      txpdoitems = DEFAULTTXPDOITEMS;
      rxpdoitems = DEFAULTTXPDOITEMS;
      initDefaultOutputSyncMgrPara();
   }
}

/**
 * SOES main function. Called to handle watchdog
 */
void soes_irq (void)
{
   DIG_process(DIG_PROCESS_WD);

}

/**
 * Initialize the SOES stack.
 */
void soes_init_irq (esc_cfg_t * config)
{
   DPRINT ("SOES (Simple Open EtherCAT Slave)\n");

   TXPDOsize = SM3_sml = sizeTXPDO();
   RXPDOsize = SM2_sml = sizeRXPDO();

   /* Init watchdog */
   watchdog_reset = config->watchdog_cnt;
   watchdog = watchdog_reset;

   /* Set application loop callback */
   application_loop_callback = config->application_loop_callback;

   /* The test is setup in the ESC handler */
   ESC_config (config);

   ESC_reset();
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
