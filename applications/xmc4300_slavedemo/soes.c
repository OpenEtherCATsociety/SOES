/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * The application.
 *
 * The application, the main loop that service EtherCAT.
 */

#include <stddef.h>

#include "esc.h"
#include "esc_coe.h"
#include "esc_eep.h"
#include "esc_hw_eep.h"

#include "utypes.h"
#include "soes.h"

#define WD_RESET           1000
#define DEFAULTTXPDOMAP    0x1a00
#define DEFAULTRXPDOMAP    0x1600
#define DEFAULTTXPDOITEMS  1
#define DEFAULTRXPDOITEMS  1

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

uint32_t            encoder_scale;
uint32_t            encoder_scale_mirror;

volatile _ESCvar  ESCvar;
_MBX              MBX[MBXBUFFERS];
_MBXcontrol       MBXcontrol[MBXBUFFERS];
uint8_t           MBXrun=0;
uint16_t          SM2_sml,SM3_sml;
_Rbuffer          Rb;
_Wbuffer          Wb;
_Cbuffer          Cb;
_App              App;
uint16_t          TXPDOsize,RXPDOsize;
int               wd_cnt = WD_RESET;
volatile uint8_t  digoutput;
volatile uint8_t  diginput;
uint16_t          txpdomap = DEFAULTTXPDOMAP;
uint16_t          rxpdomap = DEFAULTRXPDOMAP;
uint8_t           txpdoitems = DEFAULTTXPDOITEMS;
uint8_t           rxpdoitems = DEFAULTTXPDOITEMS;

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
         RXPDOsize = SM2_sml = sizeRXPDO ();
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
         TXPDOsize = SM3_sml = sizeTXPDO ();
         break;
      }
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

/** Mandatory: Hook called from the slave stack ESC_stopoutputs to act on state changes
 * forcing us to stop outputs. Here we can set them to a safe state.
 * set
 */
void APP_safeoutput (void)
{
   DPRINT ("APP_safeoutput called\n");
   Wb.LED = 0;
}
/** Mandatory: Write local process data to Sync Manager 3, Master Inputs.
 */
void TXPDO_update (void)
{
   ESC_write (SM3_sma, &Rb.button, TXPDOsize);
}
/** Mandatory: Read Sync Manager 2 to local process data, Master Outputs.
 */
void RXPDO_update (void)
{
   ESC_read (SM2_sma, &Wb.LED, RXPDOsize);
}

/** Mandatory: Function to update local I/O, call read ethercat outputs, call
 * write ethercat inputs. Implement watch-dog counter to count-out if we have
 * made state change affecting the App.state.
 */
void DIG_process (void)
{
   if (wd_cnt)
   {
      wd_cnt--;
   }
   if (App.state & APPSTATE_OUTPUT)
   {
      /* SM2 trigger ? */
      if (ESCvar.ALevent & ESCREG_ALEVENT_SM2)
      {
         ESCvar.ALevent &= ~ESCREG_ALEVENT_SM2;
         RXPDO_update ();
         wd_cnt = WD_RESET;
         /* dummy output point */
         if (Wb.LED) {
            XMC_GPIO_SetOutputHigh(P_LED);
         } else {
            XMC_GPIO_SetOutputLow(P_LED);
         }
      }
      if (!wd_cnt)
      {
         DPRINT("DIG_process watchdog tripped\n");
         ESC_stopoutput ();
         /* watchdog, invalid outputs */
         ESC_ALerror (ALERR_WATCHDOG);
         /* goto safe-op with error bit set */
         ESC_ALstatus (ESCsafeop | ESCerror);
      }
   }
   else
   {
      wd_cnt = WD_RESET;
   }
   if (App.state)
   {
      Rb.button = XMC_GPIO_GetInput(P_BTN);
      Cb.reset_counter++;
      Rb.encoder =  ESCvar.Time;
      TXPDO_update ();
   }
}

/** SOES main loop. Start by initializing the stack software followed by
 * the application loop for cyclic read the EtherCAT state and staus, update
 * of I/O.
 */
void soes_init (void)
{
   DPRINT ("SOES (Simple Open EtherCAT Slave)\n");

   ESC_reset();

   // configure I/O
   XMC_GPIO_Init(P_BTN, &gpio_config_btn);
   XMC_GPIO_Init(P_LED, &gpio_config_led);

   TXPDOsize = SM3_sml = sizeTXPDO ();
   RXPDOsize = SM2_sml = sizeRXPDO ();

   /* Setup post config hooks */
   static esc_cfg_t config =
   {
      .pre_state_change_hook = NULL,
      .post_state_change_hook = NULL
   };
   ESC_config ((esc_cfg_t *)&config);

   ESC_init (NULL);

   /*  wait until ESC is started up */
   do {
      ESC_read (ESCREG_DLSTATUS, (void *) &ESCvar.DLstatus,
                sizeof (ESCvar.DLstatus));
      ESCvar.DLstatus = etohs (ESCvar.DLstatus);
   } while ((ESCvar.DLstatus & 0x0001) == 0);

   /* reset ESC to init state */
   ESC_ALstatus (ESCinit);
   ESC_ALerror (ALERR_NONE);
   ESC_stopmbx ();
   ESC_stopinput ();
   ESC_stopoutput ();
}

void soes_task (void)
{
   /* On init restore PDO mappings to default size */
   if((ESCvar.ALstatus & 0x0f) == ESCinit)
   {
      txpdomap = DEFAULTTXPDOMAP;
      rxpdomap = DEFAULTRXPDOMAP;
      txpdoitems = DEFAULTTXPDOITEMS;
      rxpdoitems = DEFAULTTXPDOITEMS;
   }
   /* Read local time from ESC*/
   ESC_read (ESCREG_LOCALTIME, (void *) &ESCvar.Time, sizeof (ESCvar.Time));
   ESCvar.Time = etohl (ESCvar.Time);

   /* Check the state machine */
   ESC_state ();

   /* If else to two separate execution paths
    * If we're running BOOSTRAP
    *  - MailBox
    *   - FoE
    * Else we're running normal execution
    *  - MailBox
    *   - CoE
    */
    if (ESC_mbxprocess ())
    {
       ESC_coeprocess ();
       ESC_xoeprocess ();
    }
    DIG_process ();
    EEP_process ();
    EEP_hw_process();
}

