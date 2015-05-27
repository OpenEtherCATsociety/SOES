/*
 * SOES Simple Open EtherCAT Slave
 *
 * Copyright (C) 2007-2013 Arthur Ketels
 *
 * SOES is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * SOES is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * As a special exception, if other files instantiate templates or use macros
 * or inline functions from this file, or you compile this file and link it
 * with other works to produce a work based on this file, this file does not
 * by itself cause the resulting work to be covered by the GNU General Public
 * License. However the source code for this file must still be made available
 * in accordance with section (3) of the GNU General Public License.
 *
 * This exception does not invalidate any other reasons why a work based on
 * this file might be covered by the GNU General Public License.
 *
 * The EtherCAT Technology, the trade name and logo "EtherCAT" are the intellectual
 * property of, and protected by Beckhoff Automation GmbH.
 */

 /** \file
 * \brief
 * The application.
 *
 * The application, the main loop that service EtherCAT.
 */

#include <kern.h>
#include <flash_drv.h>

#include <esc.h>
#include <esc_coe.h>
#include <esc_foe.h>
#include "utypes.h"
#include "bootstrap.h"

#define WD_RESET           1000
#define DEFAULTTXPDOMAP    0x1a00
#define DEFAULTRXPDOMAP    0x1600
#define DEFAULTTXPDOITEMS  1
#define DEFAULTRXPDOITEMS  1

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


extern uint32_t local_boot_state;

static const char *spi_name = "/spi0/et1100";

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
   ESC_write (SM3_sma, &Rb.button, TXPDOsize, (void *) &ESCvar.ALevent);
}
/** Mandatory: Read Sync Manager 2 to local process data, Master Outputs.
 */
void RXPDO_update (void)
{
   ESC_read (SM2_sma, &Wb.LED, RXPDOsize, (void *) &ESCvar.ALevent);
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
         //gpio_set(GPIO_LED, Wb.LED & BIT(0));
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
      //Rb.button = gpio_get(GPIO_WAKEUP);
      Rb.button = (flash_drv_get_active_swap() && 0x8);
      Cb.reset_counter++;
      Rb.encoder =  Cb.reset_counter;
      TXPDO_update ();
   }
}

/** Optional: Hook called after state change for application specific
 * actions for specific state changes.
 */
void post_state_change_hook (uint8_t * as, uint8_t * an)
{

   /* Add specific step change hooks here */
   if ((*as == BOOT_TO_INIT) && (*an == ESCinit))
   {
      boot_inithook ();
   }
   else if((*as == INIT_TO_BOOT) && (*an & ESCerror ) == 0)
   {
      init_boothook ();
   }
}


/** SOES main loop. Start by initializing the stack software followed by
 * the application loop for cyclic read the EtherCAT state and staus, update
 * of I/O.
 */
void soes (void *arg)
{
   DPRINT ("SOES (Simple Open EtherCAT Slave)\n");

   TXPDOsize = SM3_sml = sizeTXPDO ();
   RXPDOsize = SM2_sml = sizeRXPDO ();

   /* Setup post config hooks */
   static esc_cfg_t config =
   {
      .pre_state_change_hook = NULL,
      .post_state_change_hook = post_state_change_hook
   };
   ESC_config ((esc_cfg_t *)&config);

   ESC_reset();
   ESC_init (spi_name);

   task_delay (tick_from_ms (200));

   /*  wait until ESC is started up */
   while ((ESCvar.DLstatus & 0x0001) == 0)
   {
      ESC_read (ESCREG_DLSTATUS, (void *) &ESCvar.DLstatus,
                sizeof (ESCvar.DLstatus), (void *) &ESCvar.ALevent);
      ESCvar.DLstatus = etohs (ESCvar.DLstatus);
   }

   /* Pre FoE to set up Application information */
   bootstrap_foe_init ();
   /* Init FoE */
   FOE_init();

   /* reset ESC to init state */
   ESC_ALstatus (ESCinit);
   ESC_ALerror (ALERR_NONE);
   ESC_stopmbx ();
   ESC_stopinput ();
   ESC_stopoutput ();

   DPRINT ("Application_loop GO\n");
   /* application run loop */
   while (1)
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
      ESC_read (ESCREG_LOCALTIME, (void *) &ESCvar.Time, sizeof (ESCvar.Time),
                (void *) &ESCvar.ALevent);
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
      if(local_boot_state)
      {
         if (ESC_mbxprocess ())
         {
            ESC_foeprocess ();
            ESC_xoeprocess ();
         }
         bootstrap_state ();
       }
      else
      {
         if (ESC_mbxprocess ())
         {
            ESC_coeprocess ();
            ESC_xoeprocess ();
         }
         DIG_process ();
      }
   };
}

uint8_t load1s, load5s, load10s;
void my_cyclic_callback (void * arg)
{
   while (1)
   {
      task_delay(tick_from_ms (20000));
      stats_get_load (&load1s, &load5s, &load10s);
      DPRINT ("%d:%d:%d (1s:5s:10s)\n",
               load1s, load5s, load10s);
      DPRINT ("Local bootstate: %d App.state: %d\n", local_boot_state,App.state);
      DPRINT ("AlStatus : 0x%x, AlError : 0x%x, Watchdog : %d \n", (ESCvar.ALstatus & 0x001f),ESCvar.ALerror,wd_cnt);

   }
}
