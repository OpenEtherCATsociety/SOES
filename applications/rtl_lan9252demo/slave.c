#include <stddef.h>
#include "utypes.h"
#include "esc.h"
#include "esc_coe.h"
#include "esc_foe.h"
#include "config.h"
#include "slave.h"

#define WATCHDOG_RESET_VALUE 150
#define DEFAULTTXPDOMAP    0x1a00
#define DEFAULTRXPDOMAP    0x1600
#define DEFAULTTXPDOITEMS  1
#define DEFAULTRXPDOITEMS  1

volatile _ESCvar  ESCvar;
/* Make room for largest MBX */
uint8_t           MBX[MBXBUFFERS * MAX(MBXSIZE,MBXSIZEBOOT)];
_MBXcontrol       MBXcontrol[MBXBUFFERS];
uint8_t           MBXrun=0;
uint16_t          ESC_SM2_sml, ESC_SM3_sml;
_Rbuffer          Rb;
_Wbuffer          Wb;
_Cbuffer          Cb;
_App              App;
uint16_t          TXPDOsize,RXPDOsize;
uint16_t          txpdomap = DEFAULTTXPDOMAP;
uint16_t          rxpdomap = DEFAULTRXPDOMAP;
uint8_t           txpdoitems = DEFAULTTXPDOITEMS;
uint8_t           rxpdoitems = DEFAULTTXPDOITEMS;

static int watchdog = WATCHDOG_RESET_VALUE;
static void (*application_loop_callback)(void) = NULL;

/** Function to pre-qualify the incoming SDO download.
 *
 * @param[in] index      = index of SDO download request to check
 * @param[in] sub-index  = sub-index of SDO download request to check
 * @return 1 if the SDO Download is correct. 0 If not correct.
 */
int ESC_pre_objecthandler (uint16_t index, uint8_t subindex)
{
   if ((index == 0x1c12) && (subindex > 0) && (rxpdoitems != 0))
   {
      SDO_abort (index, subindex, ABORT_READONLY);
      return 0;
   }
   if ((index == 0x1c13) && (subindex > 0) && (txpdoitems != 0))
   {
      SDO_abort (index, subindex, ABORT_READONLY);
      return 0;
   }
   return 1;
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
      RXPDOsize = SM2_sml = sizeOfPDO(RX_PDO_OBJIDX);
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
      TXPDOsize = ESC_SM3_sml = sizeOfPDO(TX_PDO_OBJIDX);
      break;
   }
   /* Handle post-write of parameter values */
   case 0x8000:
   {
      cb_post_write_Parameters(subindex);
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

   // Set safe values for Wb.LEDs
   Wb.LEDs.LED0 = 0;
   Wb.LEDs.LED1 = 0;

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
void DIG_process (void)
{
   if (watchdog > 0)
   {
      watchdog--;
   }
   if (App.state & APPSTATE_OUTPUT)
   {
      /* SM2 trigger ? */
      if (ESCvar.ALevent & ESCREG_ALEVENT_SM2)
      {
         ESCvar.ALevent &= ~ESCREG_ALEVENT_SM2;
         RXPDO_update();
         watchdog = WATCHDOG_RESET_VALUE;

         /* Set outputs */
         cb_set_LEDs();
      }
      if (watchdog <= 0)
      {
         DPRINT("DIG_process watchdog expired\n");
         ESC_stopoutput();
         /* watchdog, invalid outputs */
         ESC_ALerror (ALERR_WATCHDOG);
         /* goto safe-op with error bit set */
         ESC_ALstatus (ESCsafeop | ESCerror);
      }
   }
   else
   {
      watchdog = WATCHDOG_RESET_VALUE;
   }
   if (App.state)
   {
      /* Update inputs */
      cb_get_Buttons();
      TXPDO_update();
   }
}

/********** TODO: Generic code beyond this point ***************/

static const char *spi_name = "/spi1/lan9252";

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

/**
 * Set callback run once every loop in the SOES application loop.
 */
void set_application_loop_hook(void (*callback)(void))
{
   application_loop_callback = callback;
}

/**
 * SOES main function. It should be called periodically.
 * Reads the EtherCAT state and status. Responsible for I/O updates.
 */
void soes (void)
{
   /* On init restore PDO mappings to default size */
   if((ESCvar.ALstatus & 0x0f) == ESCinit)
   {
      txpdomap = DEFAULTTXPDOMAP;
      rxpdomap = DEFAULTRXPDOMAP;
      txpdoitems = DEFAULTTXPDOITEMS;
      rxpdoitems = DEFAULTTXPDOITEMS;
   }

   /* Read local time from ESC */
   ESC_read (ESCREG_LOCALTIME, (void *) &ESCvar.Time, sizeof (ESCvar.Time));
   ESCvar.Time = etohl (ESCvar.Time);

   /* Check the state machine */
   ESC_state();

   /* Check mailboxes */
   if (ESC_mbxprocess())
   {
      ESC_coeprocess();
      ESC_foeprocess();
      ESC_xoeprocess();
   }
   DIG_process();

   if (application_loop_callback != NULL)
   {
      (application_loop_callback)();
   }
}

/**
 * Initialize the SOES stack.
 */
void soes_init (void)
{
   DPRINT ("SOES (Simple Open EtherCAT Slave)\n");

   TXPDOsize = ESC_SM3_sml = sizeOfPDO(TX_PDO_OBJIDX);
   RXPDOsize = ESC_SM2_sml = sizeOfPDO(RX_PDO_OBJIDX);

   /* Setup config hooks */
   static esc_cfg_t config =
   {
      .use_interrupt = 0,
      .user_arg = NULL,
      .watchdog_cnt = 0,
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
      .safeoutput_override = NULL,
      .pre_object_download_hook = NULL,
      .post_object_download_hook = NULL,
      .rxpdo_override = NULL,
      .txpdo_override = NULL,
      .esc_hw_interrupt_enable = NULL,
      .esc_hw_interrupt_disable = NULL,
      .esc_hw_eep_handler = NULL
   };
   
   ESC_config (&config);
   ESC_init ((void *)spi_name);

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
