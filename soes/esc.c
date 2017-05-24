/*
 * SOES Simple Open EtherCAT Slave
 *
 * Copyright (C) 2007-2013 Arthur Ketels
 * Copyright (C) 2012-2013 rt-labs.
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
#include <string.h>
#include <cc.h>
#include "esc.h"
#include "esc_coe.h"
#include "esc_foe.h"

/** \file
 * \brief
 * Base EtherCAT functions for handling the Data Link Layer and Malilboxes
 *
 *
 * State machine and mailbox support.
 */
static esc_cfg_t * esc_cfg = NULL;

/** Write AL Status Code to the ESC.
 *
 * @param[in] errornumber   = Write an by EtherCAT specified Error number register 0x134 AL Status Code
 */
void ESC_ALerror (uint16_t errornumber)
{
   uint16_t dummy;
   ESCvar.ALerror = errornumber;
   dummy = htoes (errornumber);
   ESC_write (ESCREG_ALERROR, &dummy, sizeof (dummy));
}
/** Write AL Status to the ESC.
 *
 * @param[in] status   = Write current slave status to register 0x130 AL Status
 * reflecting actual state and error indication if present
 */
void ESC_ALstatus (uint8_t status)
{
   uint16_t dummy;
   ESCvar.ALstatus = status;
   dummy = htoes ((uint16_t) status);
   ESC_write (ESCREG_ALSTATUS, &dummy, sizeof (dummy));
}

/** Read SM Status register 0x805(+ offset to SyncManager n) to acknowledge a
 * Sync Manager event Bit 3 in ALevent. The result is not used.
 *
 * @param[in] n   = Read Sync Manager no. n
 */
void ESC_SMack (uint8_t n)
{
   uint16_t dummy;
   ESC_read (ESCREG_SM0STATUS + (n << 3), &dummy, 2);
}

/** Read SM Status register 0x805(+ offset to SyncManager n) and save the
 * result in global variable ESCvar.SM[n].
 *
 * @param[in] n   = Read Sync Manager no. n
 */
void ESC_SMstatus (uint8_t n)
{
   _ESCsm2 *sm;
   uint16_t temp;
   sm = (_ESCsm2 *) & ESCvar.SM[n];
   ESC_read (ESCREG_SM0STATUS + (n << 3), &temp, 2);
#if defined(EC_LITTLE_ENDIAN)
   sm->ActESC = temp >> 8;
   sm->Status = temp;
#endif

#if defined(EC_BIG_ENDIAN)
   sm->ActESC = temp;
   sm->Status = temp >> 8;
#endif
}

/** Write ESCvar.SM[n] data to ESC PDI control register 0x807(+ offset to SyncManager n).
 *
 * @param[in] n   = Write to Sync Manager no. n
 */
void ESC_SMwritepdi (uint8_t n)
{
   _ESCsm2 *sm;
   sm = (_ESCsm2 *) & ESCvar.SM[n];
   ESC_write (ESCREG_SM0PDI + (n << 3), &(sm->ActPDI), 1);
}

/** Write 0 to Bit0 in SM PDI control register 0x807(+ offset to SyncManager n) to Activate the Sync Manager n.
 *
 * @param[in] n   = Write to Sync Manager no. n
 */
void ESC_SMenable (uint8_t n)
{
   _ESCsm2 *sm;
   sm = (_ESCsm2 *) & ESCvar.SM[n];
   sm->ActPDI &= ~ESCREG_SMENABLE_BIT;
   ESC_SMwritepdi (n);
}
/** Write 1 to Bit0 in SM PDI control register 0x807(+ offset to SyncManager n) to De-activte the Sync Manager n.
 *
 * @param[in] n   = Write to Sync Manager no. n
 */
void ESC_SMdisable (uint8_t n)
{
   _ESCsm2 *sm;
   sm = (_ESCsm2 *) & ESCvar.SM[n];
   sm->ActPDI |= ESCREG_SMENABLE_BIT;
   ESC_SMwritepdi (n);
}
/** Read Configured Station Address register 0x010 assigned by the Master.
 *
 */
void ESC_address (void)
{
   ESC_read (ESCREG_ADDRESS, (void *) &ESCvar.address, sizeof (ESCvar.address));
   ESCvar.address = etohs (ESCvar.address);
}

/** Read Watchdog Status register 0x440. Result Bit0 0= Expired, 1= Active or disabled.
 *
 * @return value of register Watchdog Status.
 */
uint8_t ESC_WDstatus (void)
{
   uint16_t wdstatus;
   ESC_read (ESCREG_WDSTATUS, &wdstatus, 2);
   wdstatus = etohs (wdstatus);
   return (uint8_t) wdstatus;
}

/** Check mailbox status by reading all SyncManager 0 and 1 data. The read values
 * are compared with local definitions for SM Physical Address, SM Length and SM Control.
 * If we check fails we disable Mailboxes by disabling SyncManager 0 and 1 and return
 * state Init with Error flag set.
 *
 * @param[in] state   = Current state request read from ALControl 0x0120
 * @return if all Mailbox values is correct we return incoming state request, otherwise
 * we return state Init with Error flag set.
 */
uint8_t ESC_checkmbx (uint8_t state)
{
   _ESCsm2 *SM;
   ESC_read (ESCREG_SM0, (void *) &ESCvar.SM[0], sizeof (ESCvar.SM[0]));
   ESC_read (ESCREG_SM1, (void *) &ESCvar.SM[1], sizeof (ESCvar.SM[1]));
   SM = (_ESCsm2 *) & ESCvar.SM[0];
   if ((etohs (SM->PSA) != MBX0_sma) || (etohs (SM->Length) != MBX0_sml)
       || (SM->Command != MBX0_smc) || (ESCvar.SM[0].ECsm == 0))
   {
      ESCvar.SMtestresult = SMRESULT_ERRSM0;
      ESC_SMdisable (0);
      ESC_SMdisable (1);
      return (uint8_t) (ESCinit | ESCerror);      //fail state change
   }
   SM = (_ESCsm2 *) & ESCvar.SM[1];
   if ((etohs (SM->PSA) != MBX1_sma) || (etohs (SM->Length) != MBX1_sml)
       || (SM->Command != MBX1_smc) || (ESCvar.SM[1].ECsm == 0))
   {
      ESCvar.SMtestresult = SMRESULT_ERRSM1;
      ESC_SMdisable (0);
      ESC_SMdisable (1);
      return ESCinit | ESCerror;        //fail state change
   }
   return state;
}
/** Try to start mailboxes for current ALControl state request by enabling SyncManager 0 and 1.
 * If all mailbox settings is correct we return incoming state request, otherwise
 * we return state Init with Error flag set and update local ALerror with code 0x16 Invalid
 * mailbox configuration.
 *
 * @param[in] state   = Current state request read from ALControl 0x0120
 * @return if all Mailbox values is correct we return incoming state, otherwise
 * we return state Init with Error flag set.
 */
uint8_t ESC_startmbx (uint8_t state)
{
   ESC_SMenable (0);
   ESC_SMenable (1);
   ESC_SMstatus (0);
   ESC_SMstatus (1);
   if ((state = ESC_checkmbx (state)) & ESCerror)
   {
      ESC_ALerror (ALERR_INVALIDMBXCONFIG);
      MBXrun = 0;
   }
   else
   {
      ESCvar.toggle = ESCvar.SM[1].ECrep;       //sync repeat request toggle state
      MBXrun = 1;
   }
   return state;
}
/** Check boostrap mailbox status by reading all SyncManager 0 and 1 data. The read values
 * are compared with local definitions for SM Physical Address, SM Length and SM Control.
 * If we check fails we disable Mailboxes by disabling SyncManager 0 and 1 and return
 * state Init with Error flag set.
 *
 * @param[in] state   = Current state request read from ALControl 0x0120
 * @return if all Mailbox values is correct we return incoming state request, otherwise
 * we return state Init with Error flag set.
 */
uint8_t ESC_checkmbxboot (uint8_t state)
{
   _ESCsm2 *SM;
   ESC_read (ESCREG_SM0, (void *) &ESCvar.SM[0], sizeof (ESCvar.SM[0]));
   ESC_read (ESCREG_SM1, (void *) &ESCvar.SM[1], sizeof (ESCvar.SM[1]));
   SM = (_ESCsm2 *) & ESCvar.SM[0];
   if ((etohs (SM->PSA) != MBX0_sma_b) || (etohs (SM->Length) != MBX0_sml_b)
       || (SM->Command != MBX0_smc_b) || (ESCvar.SM[0].ECsm == 0))
   {
      ESCvar.SMtestresult = SMRESULT_ERRSM0;
      ESC_SMdisable (0);
      ESC_SMdisable (1);
      return (uint8_t) (ESCinit | ESCerror);      //fail state change
   }
   SM = (_ESCsm2 *) & ESCvar.SM[1];
   if ((etohs (SM->PSA) != MBX1_sma_b) || (etohs (SM->Length) != MBX1_sml_b)
       || (SM->Command != MBX1_smc_b) || (ESCvar.SM[1].ECsm == 0))
   {
      ESCvar.SMtestresult = SMRESULT_ERRSM1;
      ESC_SMdisable (0);
      ESC_SMdisable (1);
      return (uint8_t) (ESCinit | ESCerror);      //fail state change
   }
   return state;
}

/** Try to start bootstrap mailboxes for current ALControl state request by enabling SyncManager 0 and 1.
 * If all mailbox settings is correct we return incoming state request, otherwise
 * we return state Init with Error flag set and update local ALerror with code 0x16 Invalid
 * mailbox configuration.
 *
 * @param[in] state   = Current state request read from ALControl 0x0120
 * @return if all Mailbox values is correct we return incoming state, otherwise
 * we return state Init with Error flag set.
 */
uint8_t ESC_startmbxboot (uint8_t state)
{
   ESC_SMenable (0);
   ESC_SMenable (1);
   ESC_SMstatus (0);
   ESC_SMstatus (1);
   if ((state = ESC_checkmbxboot (state)) & ESCerror)
   {
      ESC_ALerror (ALERR_INVALIDBOOTMBXCONFIG);
      MBXrun = 0;
   }
   else
   {
      ESCvar.toggle = ESCvar.SM[1].ECrep;       //sync repeat request toggle state
      MBXrun = 1;
   }
   return state;
}

/** Stop mailboxes by disabling SyncManager 0 and 1. Clear local mailbox variables
 * stored in ESCvar.
 */
void ESC_stopmbx (void)
{
   uint8_t n;
   MBXrun = 0;
   ESC_SMdisable (0);
   ESC_SMdisable (1);
   for (n = 0; n < MBXBUFFERS; n++)
   {
      MBXcontrol[n].state = MBXstate_idle;
   }
   ESCvar.mbxoutpost = 0;
   ESCvar.mbxbackup = 0;
   ESCvar.xoe = 0;
   ESCvar.mbxfree = 1;
   ESCvar.toggle = 0;
   ESCvar.mbxincnt = 0;
   ESCvar.segmented = 0;
   ESCvar.frags = 0;
   ESCvar.fragsleft = 0;
   ESCvar.txcue = 0;
}

/** Read Receive mailbox and store data in local ESCvar.MBX variable.
 * Combined function for bootstrap and other states. State check decides
 * which one to read.
 */
void ESC_readmbx (void)
{
   _MBX *MB = &MBX[0];
   uint16_t length;

   if (ESCvar.ALstatus == ESCboot)
   {
      ESC_read (MBX0_sma_b, MB, MBXHSIZE);
      length = etohs (MB->header.length);

      if (length > (MBX0_sml_b - MBXHSIZE))
      {
         length = MBX0_sml_b - MBXHSIZE;
      }
      ESC_read (MBX0_sma_b + MBXHSIZE, &(MB->b[0]), length);
      if (length + MBXHSIZE < MBX0_sml_b)
      {
         ESC_read (MBX0_sme_b, &length, 1);
      }
   }
   else
   {
      ESC_read (MBX0_sma, MB, MBXHSIZE);
      length = etohs (MB->header.length);

      if (length > (MBX0_sml - MBXHSIZE))
      {
         length = MBX0_sml - MBXHSIZE;
      }
      ESC_read (MBX0_sma + MBXHSIZE, &(MB->b[0]), length);
      if (length + MBXHSIZE < MBX0_sml)
      {
         ESC_read (MBX0_sme, &length, 1);
      }
   }
   MBXcontrol[0].state = MBXstate_inclaim;
}
/** Write local mailbox buffer ESCvar.MBX[n] to Send mailbox.
 * Combined function for bootstrap and other states. State check decides
 * which one to write.
 *
 * @param[in] n   = Which local mailbox buffer n to send.
 */
void ESC_writembx (uint8_t n)
{
   _MBX *MB = &MBX[n];
   uint8_t dummy = 0;
   uint16_t length;
   length = etohs (MB->header.length);
   if (ESCvar.ALstatus == ESCboot)
   {
      if (length > (MBX1_sml_b - MBXHSIZE))
      {
         length = MBX1_sml_b - MBXHSIZE;
      }
      ESC_write (MBX1_sma_b, MB, MBXHSIZE + length);
      if (length + MBXHSIZE < MBX1_sml_b)
      {
         ESC_write (MBX1_sme_b, &dummy, 1);
      }
   }
   else
   {
      if (length > (MBX1_sml - MBXHSIZE))
      {
         length = MBX1_sml - MBXHSIZE;
      }
      ESC_write (MBX1_sma, MB, MBXHSIZE + length);
      if (length + MBXHSIZE < MBX1_sml)
      {
         ESC_write (MBX1_sme, &dummy, 1);
      }
   }
   ESCvar.mbxfree = 0;
}

/** TBD
 */
void ESC_ackmbxread (void)
{
   uint8_t dummy = 0;
   if (ESCvar.ALstatus == ESCboot)
   {
      ESC_write (MBX1_sma_b, &dummy, 1);
   }
   else
   {
      ESC_write (MBX1_sma, &dummy, 1);
   }
   ESCvar.mbxfree = 1;
}

/** Allocate and prepare a mailbox buffer. Take the first Idle buffer from the End.
 * Set Mailbox control state to be used for outbox and fill the mailbox buffer with
 * address master and mailbox next CNT value between 1-7.
 *
 * @return The index of Mailbox buffer prepared for outbox. IF no buffer is available return 0.
 */
uint8_t ESC_claimbuffer (void)
{
   _MBX *MB;
   uint8_t n = MBXBUFFERS - 1;
   while ((n > 0) && (MBXcontrol[n].state))
   {
      n--;
   }
   if (n)
   {
      MBXcontrol[n].state = MBXstate_outclaim;
      MB = &MBX[n];
      ESCvar.mbxcnt = (++ESCvar.mbxcnt) & 0x07;
      if (ESCvar.mbxcnt == 0)
      {
         ESCvar.mbxcnt = 1;
      }
      MB->header.address = htoes (0x0000);      // destination is master
      MB->header.channel = 0;
      MB->header.priority = 0;
      MB->header.mbxcnt = ESCvar.mbxcnt;
      ESCvar.txcue++;
   }
   return n;
}

/** Look for any present requests for posting to the outbox.
 *
 * @return the index of Mailbox buffer ready to be posted.
 */
uint8_t ESC_outreqbuffer (void)
{
   uint8_t n = MBXBUFFERS - 1;
   while ((n > 0) && (MBXcontrol[n].state != MBXstate_outreq))
   {
      n--;
   }
   return n;
}
/** Allocate and prepare a mailbox buffer for sending an error message. Take the first Idle
 * buffer from the end. Set Mailbox control state to be used for outbox and fill the mailbox
 * buffer with error information.
 *
 * @param[in] n   = Error number to be sent in mailbox error message.
 */
void MBX_error (uint16_t error)
{
   uint8_t MBXout;
   _MBXerr *mbxerr;
   MBXout = ESC_claimbuffer ();
   if (MBXout)
   {
      mbxerr = (_MBXerr *) & MBX[MBXout];
      mbxerr->mbxheader.length = htoes ((uint16_t) 0x04);
      mbxerr->mbxheader.mbxtype = MBXERR;
      mbxerr->type = htoes ((uint16_t) 0x01);
      mbxerr->detail = htoes (error);
      MBXcontrol[MBXout].state = MBXstate_outreq;
   }
}

/** Mailbox routine for implementing the low-level part of the mailbox protocol
 * used by Application Layers running on-top of mailboxes. It takes care of sending
 * a mailbox, re-sending a mailbox, reading a mailbox and handles a mailbox full event.
 *
 * @return =0 if nothing to do. =1 if something to be handled by mailbox protocols.
 */
uint8_t ESC_mbxprocess (void)
{
   uint8_t mbxhandle = 0;
   _MBX *MB = &MBX[0];

   if (!MBXrun)
   {
      /* nothing to do */
      return 0;
   }

   /* SM0/1 access or SMn change event */
   if (ESCvar.ALevent & ESCREG_ALEVENT_SM_MASK)
   {
      ESC_SMstatus (0);
      ESC_SMstatus (1);
   }

   /* outmbx read by master */
   if (ESCvar.mbxoutpost && ESCvar.SM[1].IntR)
   {
      ESC_ackmbxread ();
      /* dispose old backup */
      if (ESCvar.mbxbackup)
      {
         MBXcontrol[ESCvar.mbxbackup].state = MBXstate_idle;
      }
      /* if still to do */
      if (MBXcontrol[ESCvar.mbxoutpost].state == MBXstate_again)
      {
         ESC_writembx (ESCvar.mbxoutpost);
      }
      /* create new backup */
      MBXcontrol[ESCvar.mbxoutpost].state = MBXstate_backup;
      ESCvar.mbxbackup = ESCvar.mbxoutpost;
      ESCvar.mbxoutpost = 0;
      /* Do we have any ongoing protocol transfers, return 1 */
      if(ESCvar.xoe > 0)
      {
         return 1;
      }
      return 0;
   }

   /* repeat request */
   if (ESCvar.SM[1].ECrep != ESCvar.toggle)
   {
      if (ESCvar.mbxoutpost || ESCvar.mbxbackup)
      {
         /* if outmbx empty */
         if (!ESCvar.mbxoutpost)
         {
            /* use backup mbx */
            ESC_writembx (ESCvar.mbxbackup);
         }
         else
         {
            /* reset mailbox */
            ESC_SMdisable (1);
            /* have to resend later */
            MBXcontrol[ESCvar.mbxoutpost].state = MBXstate_again;
            /* activate mailbox */
            ESC_SMenable (1);
            /* use backup mbx */
            ESC_writembx (ESCvar.mbxbackup);
         }
         ESCvar.toggle = ESCvar.SM[1].ECrep;
         ESCvar.SM[1].PDIrep = ESCvar.toggle;
         ESC_SMwritepdi (1);
      }
      return 0;
   }

   /* if the outmailbox is free check if we have something to send */
   if (ESCvar.txcue && (ESCvar.mbxfree || !ESCvar.SM[1].MBXstat))
   {
      /* check out request mbx */
      mbxhandle = ESC_outreqbuffer ();
      /* outmbx empty and outreq mbx available */
      if (mbxhandle)
      {
         ESC_writembx (mbxhandle);
         /* change state */
         MBXcontrol[mbxhandle].state = MBXstate_outpost;
         ESCvar.mbxoutpost = mbxhandle;
         if (ESCvar.txcue)
         {
            ESCvar.txcue--;
         }
      }
   }

   /* read mailbox if full and no xoe in progress */
   if (ESCvar.SM[0].MBXstat && !MBXcontrol[0].state && !ESCvar.mbxoutpost
       && !ESCvar.xoe)
   {
      ESC_readmbx ();
      ESCvar.SM[0].MBXstat = 0;
      if (etohs (MB->header.length) == 0)
      {
         MBX_error (MBXERR_INVALIDHEADER);
         /* drop mailbox */
         MBXcontrol[0].state = MBXstate_idle;
      }
      if ((MB->header.mbxcnt != 0) && (MB->header.mbxcnt == ESCvar.mbxincnt))
      {
         /* drop mailbox */
         MBXcontrol[0].state = MBXstate_idle;
      }
      ESCvar.mbxincnt = MB->header.mbxcnt;
      return 1;
   }

   /* ack changes in non used SM */
   if (ESCvar.ALevent & ESCREG_ALEVENT_SMCHANGE)
   {
      ESC_SMack (4);
      ESC_SMack (5);
      ESC_SMack (6);
      ESC_SMack (7);
   }

   return 0;
}
/** Handler for incorrect or unsupported mailbox data. Write error response
 * in Mailbox.
 */
void ESC_xoeprocess (void)
{
   _MBXh *mbh;
   if (!MBXrun)
   {
      return;
   }
   if (!ESCvar.xoe && (MBXcontrol[0].state == MBXstate_inclaim))
   {
      mbh = (_MBXh *) & MBX[0];
      if ((mbh->mbxtype == 0) || (etohs (mbh->length) == 0))
      {
         MBX_error (MBXERR_INVALIDHEADER);
      }
      else
      {
         MBX_error (MBXERR_UNSUPPORTEDPROTOCOL);
      }
      /* mailbox type not supported, drop mailbox */
      MBXcontrol[0].state = MBXstate_idle;
   }
}

/** Validate the values of Sync Manager 2 & 3 that the current ESC values is
 * equal to configured and calculated local values.
 *
 * @param[in] state   = Requested state.
 * @return = incoming state request if every thing checks out OK. = state (PREOP | ERROR)  if something isn't correct.
 */
uint8_t ESC_checkSM23 (uint8_t state)
{
   _ESCsm2 *SM;
   ESC_read (ESCREG_SM2, (void *) &ESCvar.SM[2], sizeof (ESCvar.SM[2]));
   ESC_read (ESCREG_SM3, (void *) &ESCvar.SM[3], sizeof (ESCvar.SM[3]));
   SM = (_ESCsm2 *) & ESCvar.SM[2];
   if ((etohs (SM->PSA) != SM2_sma) || (etohs (SM->Length) != SM2_sml)
       || (SM->Command != SM2_smc) || !(SM->ActESC & SM2_act))
   {
      ESCvar.SMtestresult = SMRESULT_ERRSM2;
      /* fail state change */
      return (ESCpreop | ESCerror);
   }
   SM = (_ESCsm2 *) & ESCvar.SM[3];
   if ((etohs (SM->PSA) != SM3_sma) || (etohs (SM->Length) != SM3_sml)
       || (SM->Command != SM3_smc) || !(SM->ActESC & SM3_act))
   {
      ESCvar.SMtestresult = SMRESULT_ERRSM3;
      /* fail state change */
      return (ESCpreop | ESCerror);
   }
   return state;
}

/** Function trying to enable start updating the process data inputs. It calls the check SM 2 & 3
 * routine, based on the result from there if enables or disables the Input SyncManager, in addition
 * it updates the ALStatusCode case something didn't pass the check.
 *
 * @param[in] state   = Requested state.
 * @return = state, incoming state request if every thing checks out OK. =state (PREOP | ERROR) if something isn't correct.
 */
uint8_t ESC_startinput (uint8_t state)
{
   state = ESC_checkSM23 (state);
   if (state != (ESCpreop | ESCerror))
   {
      ESC_SMenable (3);
      App.state = APPSTATE_INPUT;
   }
   else
   {
      ESC_SMdisable (2);
      ESC_SMdisable (3);
      if (ESCvar.SMtestresult & SMRESULT_ERRSM3)
      {
         ESC_ALerror (ALERR_INVALIDINPUTSM);
      }
      else
      {
         ESC_ALerror (ALERR_INVALIDOUTPUTSM);
      }
   }
   return state;
}

/** Unconditional stop of updating inputs by disabling Sync Manager 2 & 3.
 * Set the App.state to APPSTATE_IDLE.
 *
 */
void ESC_stopinput (void)
{
   App.state = APPSTATE_IDLE;
   ESC_SMdisable (3);
   ESC_SMdisable (2);
}


/** Unconditional start of updating outputs by enabling Sync Manager 2.
 * Set the App.state to APPSTATE_OUTPUT.
 *
 * @param[in] state   = Not used.
 * @return = state unchanged.
 *
 */
uint8_t ESC_startoutput (uint8_t state)
{
   ESC_SMenable (2);
   App.state |= APPSTATE_OUTPUT;
   return state;
}

/** Unconditional stop of updating outputs by disabling Sync Manager 2.
 * Set the App.state to APPSTATE_ONPUT. Call application hook APP_safeoutput
 * letting the user to set safe state values on outputs.
 *
 */
void ESC_stopoutput (void)
{
   App.state &= APPSTATE_INPUT;
   ESC_SMdisable (2);
   APP_safeoutput ();
}
/** The state handler acting on ALControl Bit(0) and SyncManager Activation BIT(4)
 * events in the Al Event Request register 0x220.
 *
 */
void ESC_state (void)
{
   uint8_t ac, an, as, ax, ax23;
   uint8_t handle_smchanged = 0;

   /* Do we have a state change request pending */
   if (ESCvar.ALevent & ESCREG_ALEVENT_CONTROL)
   {
      ESC_read (ESCREG_ALCONTROL, (void *) &ESCvar.ALcontrol,
                sizeof (ESCvar.ALcontrol));
      ESCvar.ALcontrol = etohs (ESCvar.ALcontrol);
   }
   /* Have at least on Sync Manager  changed */
   else if (ESCvar.ALevent & ESCREG_ALEVENT_SMCHANGE)
   {
      handle_smchanged  = 1;
   }
   else
   {
      /* nothing to do */
      return;
   }
   /* Mask state request bits + Error ACK */
   ac = ESCvar.ALcontrol & ESCREG_AL_STATEMASK;
   as = ESCvar.ALstatus & ESCREG_AL_STATEMASK;
   an = as;
   if (((ac & ESCerror) || (ac == ESCinit)))
   {
      /* if error bit confirmed reset */
      ac &= ESCREG_AL_ERRACKMASK;
      an &= ESCREG_AL_ERRACKMASK;
   }
   /* Enter SM changed handling for all steps but Init and Boot when Mailboxes
    * is up and running
    */
   if (handle_smchanged && (as & ESCREG_AL_ALLBUTINITMASK) &&
       !(as == ESCboot) && MBXrun)
   {
      /* Validate Sync Managers, reading the Activation register will
       * acknowledge the SyncManager Activation event making us enter
       * this execution path.
       */
      ax = ESC_checkmbx (as);
      ax23 = ESC_checkSM23 (as);
      if ((an & ESCerror) && !(ac & ESCerror))
      {
         /* if in error then stay there */
         return;
      }
      /* Have we been forced to step down to INIT we will stop mailboxes,
       * update AL Status Code and exit ESC_state
       */
      if (ax == (ESCinit | ESCerror))
      {
         /* If we have activated Inputs and Outputs we need to disable them */
         if (App.state)
         {
            ESC_stopoutput ();
            ESC_stopinput ();
         }
         /* Stop mailboxes and update ALStatus code */
         ESC_stopmbx ();
         ESC_ALerror (ALERR_INVALIDMBXCONFIG);
         MBXrun = 0;
         ESC_ALstatus (ax);
         return;
      }
      /* Have we been forced to step down to PREOP we will stop inputs
       * and outputs, update AL Status Code and exit ESC_state
       */
      if ((App.state) && (ax23 == (ESCpreop | ESCerror)))
      {
         ESC_stopoutput ();
         ESC_stopinput ();
         if (ESCvar.SMtestresult & SMRESULT_ERRSM3)
         {
            ESC_ALerror (ALERR_INVALIDINPUTSM);
         }
         else
         {
            ESC_ALerror (ALERR_INVALIDOUTPUTSM);
         }
         ESC_ALstatus (ax23);
         return;
      }
   }

   /* Error state not acked, leave original */
   if ((an & ESCerror) && !(ac & ESCerror))
   {
      return;
   }

   /* Mask high bits ALcommand, low bits ALstatus */
   as = (ac << 4) | (as & 0x0f);

   /* Call post state change hook case it have been configured  */
   if ((esc_cfg != NULL) && esc_cfg->pre_state_change_hook)
   {
      esc_cfg->pre_state_change_hook (&as, &an);
   }

   /* Switch through the state change requested via AlControl from
    * current state read in AL status
    */
   switch (as)
   {
      case INIT_TO_INIT:
      case PREOP_TO_PREOP:
      case OP_TO_OP:
      {
         break;
      }
      case INIT_TO_PREOP:
      {
         /* get station address */
         ESC_address ();
         an = ESC_startmbx (ac);
         break;
      }
      case INIT_TO_BOOT:
      case BOOT_TO_BOOT:
      {
         /* get station address */
         ESC_address ();
         an = ESC_startmbxboot (ac);
         break;
      }
      case INIT_TO_SAFEOP:
      case INIT_TO_OP:
      {
         an = ESCinit | ESCerror;
         ESC_ALerror (ALERR_INVALIDSTATECHANGE);
         break;
      }
      case OP_TO_INIT:
      {
         ESC_stopoutput ();
      }
      case SAFEOP_TO_INIT:
      {
         ESC_stopinput ();
      }
      case PREOP_TO_INIT:
      {
         ESC_stopmbx ();
         an = ESCinit;
         break;
      }
      case BOOT_TO_INIT:
      {
         ESC_stopmbx ();
         an = ESCinit;
         break;
      }
      case PREOP_TO_BOOT:
      case BOOT_TO_PREOP:
      case BOOT_TO_SAFEOP:
      case BOOT_TO_OP:
      {
         an = ESCpreop | ESCerror;
         ESC_ALerror (ALERR_INVALIDSTATECHANGE);
         break;
      }
      case PREOP_TO_SAFEOP:
      case SAFEOP_TO_SAFEOP:
      {
         SM2_sml = sizeRXPDO ();
         SM3_sml = sizeTXPDO ();
         an = ESC_startinput (ac);
         if (an == ac)
         {
            ESC_SMenable (2);
         }
         break;
      }
      case PREOP_TO_OP:
      {
         an = ESCpreop | ESCerror;
         ESC_ALerror (ALERR_INVALIDSTATECHANGE);
         break;
      }
      case OP_TO_PREOP:
      {
         ESC_stopoutput ();
      }
      case SAFEOP_TO_PREOP:
      {
         ESC_stopinput ();
         an = ESCpreop;
         break;
      }
      case SAFEOP_TO_BOOT:
      {
         an = ESCsafeop | ESCerror;
         ESC_ALerror (ALERR_INVALIDSTATECHANGE);
         break;
      }
      case SAFEOP_TO_OP:
      {
         an = ESC_startoutput (ac);
         break;
      }
      case OP_TO_BOOT:
      {
         an = ESCsafeop | ESCerror;
         ESC_ALerror (ALERR_INVALIDSTATECHANGE);
         ESC_stopoutput ();
         break;
      }
      case OP_TO_SAFEOP:
      {
         an = ESCsafeop;
         ESC_stopoutput ();
         break;
      }
      default:
      {
         if (an == ESCop)
         {
            ESC_stopoutput ();
            an = ESCsafeop;
         }
         if (as == ESCsafeop)
         {
            ESC_stopinput ();
         }
         an |= ESCerror;
         ESC_ALerror (ALERR_UNKNOWNSTATE);
         break;
      }
   }

   /* Call post state change hook case it have been configured  */
   if ((esc_cfg != NULL) && esc_cfg->post_state_change_hook)
   {
      esc_cfg->post_state_change_hook (&as, &an);
   }

   if (!(an & ESCerror) && (ESCvar.ALerror))
   {
      /* clear error */
      ESC_ALerror (ALERR_NONE);
   }

   ESC_ALstatus (an);

}
/** Function copying the application configuration variable
 * to the stack local pointer variable.
 *
 * @param[in] cfg   = Pointer to by the Application static declared
 * configuration variable holding application specific details. Ex. post- and
 * pre state change hooks
 */
void ESC_config (esc_cfg_t * cfg)
{
   esc_cfg = cfg;
}
