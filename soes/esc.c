/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
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

/** Write AL Status and AL Status code to the ESC.
 *  Call pre- and poststate change hook
 *
 * @param[in] status   = Write current slave status to register 0x130 AL Status
 * reflecting actual state and error indication if present
 * @param[in] errornumber   = Write an by EtherCAT specified Error number
 * register 0x134 AL Status Code
 */
void ESC_ALstatusgotoerror (uint8_t status, uint16_t errornumber)
{
   uint8_t an, as;

   if(status & ESCop)
   {
      /* Erroneous input, ignore */
      return;
   }
   /* Mask error ack of current state */
   as = ESCvar.ALstatus & ESCREG_AL_ERRACKMASK;
   an = as;
   /* Set the state transition, new state in high bits and old in bits  */
   as = ((status & ESCREG_AL_ERRACKMASK) << 4) | (as & 0x0f);
   /* Call post state change hook case it have been configured  */
   if (ESCvar.pre_state_change_hook != NULL)
   {
      ESCvar.pre_state_change_hook (&as, &an);
   }
   /* Stop outputs if active */
   if ((CC_ATOMIC_GET(ESCvar.App.state) & APPSTATE_OUTPUT) > 0)
   {
      ESC_stopoutput();
   }
   ESC_ALerror(errornumber);
   ESC_ALstatus(status);
   an = status;
   /* Call post state change hook case it have been configured  */
   if (ESCvar.post_state_change_hook != NULL)
   {
      ESCvar.post_state_change_hook (&as, &an);
   }
}

/** Write ALeventMask register 0x204.
 *
 * @param[in] n   = AL Event Mask
 */
void ESC_ALeventmaskwrite (uint32_t mask)
{
   uint32_t aleventmask;
   aleventmask = htoel(mask);
   ESC_write (ESCREG_ALEVENTMASK, &aleventmask, sizeof(aleventmask));
}

/** Read AleventMask register 0x204.
 *
 * @return value of register AL Event Mask
 */
uint32_t ESC_ALeventmaskread (void)
{
   uint32_t aleventmask;

   ESC_read (ESCREG_ALEVENTMASK, &aleventmask, sizeof(aleventmask));
   return htoel(aleventmask);
}

/** Write ALevent register 0x220.
 *
 * @param[in] n   = AL Event Mask
 */
void ESC_ALeventwrite (uint32_t event)
{
   uint32_t alevent;
   alevent = htoel(event);
   ESC_write (ESCREG_ALEVENT, &alevent, sizeof(alevent));
}

/** Read Alevent register 0x220.
 *
 * @return value of register AL Event Mask
 */
uint32_t ESC_ALeventread (void)
{
   uint32_t alevent;
   ESC_read (ESCREG_ALEVENT, &alevent, sizeof(alevent));
   return htoel(alevent);
}

/** Read SM Activate register 0x806(+ offset to SyncManager n) to acknowledge a
 * Sync Manager event Bit 3 in ALevent. The result is not used.
 *
 * @param[in] n   = Read Sync Manager no. n
 */
void ESC_SMack (uint8_t n)
{
   uint8_t dummy;
   ESC_read (ESCREG_SM0ACTIVATE + (n << 3), &dummy, 1);
}

/** Read SM Status register 0x805(+ offset to SyncManager n) and save the
 * result in global variable ESCvar.SM[n].
 *
 * @param[in] n   = Read Sync Manager no. n
 */
void ESC_SMstatus (uint8_t n)
{
   _ESCsm2 *sm;
   sm = (_ESCsm2 *)&ESCvar.SM[n];
   ESC_read (ESCREG_SM0STATUS + (n << 3), &(sm->Status), 1);
}

/** Write ESCvar.SM[n] data to ESC PDI control register 0x807(+ offset to SyncManager n).
 *
 * @param[in] n   = Write to Sync Manager no. n
 */
void ESC_SMwritepdi (uint8_t n)
{
   _ESCsm2 *sm;
   sm = (_ESCsm2 *)&ESCvar.SM[n];
   ESC_write (ESCREG_SM0PDI + (n << 3), &(sm->ActPDI), 1);
}

/** Write 0 to Bit0 in SM PDI control register 0x807(+ offset to SyncManager n) to Activate the Sync Manager n.
 *
 * @param[in] n   = Write to Sync Manager no. n
 */
void ESC_SMenable (uint8_t n)
{
   _ESCsm2 *sm;
   sm = (_ESCsm2 *)&ESCvar.SM[n];
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
   sm = (_ESCsm2 *)&ESCvar.SM[n];
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

/** Read SYNC Out Unit activation registers 0x981
 *
 * @return value of register Activation.
 */
uint8_t ESC_SYNCactivation (void)
{
   uint8_t activation;
   ESC_read (ESCREG_SYNC_ACT, &activation, sizeof(activation));
   return activation;
}

/** Read SYNC0 cycle time
 *
 * @return value of register SYNC0 cycle time
 */
uint32_t ESC_SYNC0cycletime (void)
{
   uint32_t cycletime;
   ESC_read (ESCREG_SYNC0_CYCLE_TIME, &cycletime, sizeof(cycletime));
   cycletime = etohl (cycletime);
   return cycletime;
}

/** Read SYNC1 cycle time
 *
 * @return value of register SYNC1 cycle time
 */
uint32_t ESC_SYNC1cycletime (void)
{
   uint32_t cycletime;
   ESC_read (ESCREG_SYNC1_CYCLE_TIME, &cycletime, 4);
   cycletime = etohl (cycletime);
   return cycletime;
}


/** Validate the DC values if the SYNC unit is activated.
 *
 * @return = 0 if OK, else ERROR code to be set by caller.
 */
uint16_t ESC_checkDC (void)
{
   uint16_t ret = 0;

   uint8_t sync_act = ESC_SYNCactivation();
   /* Do we need to check sync settings? */
   if((sync_act & (ESCREG_SYNC_ACT_ACTIVATED | ESCREG_SYNC_AUTO_ACTIVATED)) > 0)
   {
      /* Trigger a by the application given DC check handler, return error if
       *  non is given
       */
      ret = ALERR_DCINVALIDSYNCCFG;
      if(ESCvar.esc_check_dc_handler != NULL)
      {
         ret = (ESCvar.esc_check_dc_handler)();
      }
   }
   else
   {
      ESCvar.dcsync = 0;
      ESCvar.synccounter = 0;
   }

   return ret;
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
   if ((etohs (SM->PSA) != ESC_MBX0_sma) || (etohs (SM->Length) != ESC_MBX0_sml)
       || (SM->Command != ESC_MBX0_smc) || (ESCvar.SM[0].ECsm == 0))
   {
      ESCvar.SMtestresult = SMRESULT_ERRSM0;
      ESC_SMdisable (0);
      ESC_SMdisable (1);
      return (uint8_t) (ESCinit | ESCerror);      //fail state change
   }
   SM = (_ESCsm2 *) & ESCvar.SM[1];
   if ((etohs (SM->PSA) != ESC_MBX1_sma) || (etohs (SM->Length) != ESC_MBX1_sml)
       || (SM->Command != ESC_MBX1_smc) || (ESCvar.SM[1].ECsm == 0))
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
   /* Assign SM settings */
   ESCvar.activembxsize = MBXSIZE;
   ESCvar.activemb0 = &ESCvar.mb[0];
   ESCvar.activemb1 = &ESCvar.mb[1];


   ESC_SMenable (0);
   ESC_SMenable (1);
   ESC_SMstatus (0);
   ESC_SMstatus (1);
   if ((state = ESC_checkmbx (state)) & ESCerror)
   {
      ESC_ALerror (ALERR_INVALIDMBXCONFIG);
      ESCvar.MBXrun = 0;
   }
   else
   {
      ESCvar.toggle = ESCvar.SM[1].ECrep;       //sync repeat request toggle state
      ESCvar.MBXrun = 1;
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
   /* Assign SM settings */
   ESCvar.activembxsize = MBXSIZEBOOT;
   ESCvar.activemb0 = &ESCvar.mbboot[0];
   ESCvar.activemb1 = &ESCvar.mbboot[1];

   ESC_SMenable (0);
   ESC_SMenable (1);
   ESC_SMstatus (0);
   ESC_SMstatus (1);
   if ((state = ESC_checkmbx (state)) & ESCerror)
   {
      ESC_ALerror (ALERR_INVALIDBOOTMBXCONFIG);
      ESCvar.MBXrun = 0;
   }
   else
   {
      ESCvar.toggle = ESCvar.SM[1].ECrep;       //sync repeat request toggle state
      ESCvar.MBXrun = 1;
   }
   return state;
}

/** Stop mailboxes by disabling SyncManager 0 and 1. Clear local mailbox variables
 * stored in ESCvar.
 */
void ESC_stopmbx (void)
{
   uint8_t n;
   ESCvar.MBXrun = 0;
   ESC_SMdisable (0);
   ESC_SMdisable (1);
   for (n = 0; n < ESC_MBXBUFFERS; n++)
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
   _MBX *MB = (_MBX *)&MBX[0];
   uint16_t length;

   ESC_read (ESC_MBX0_sma, MB, ESC_MBXHSIZE);
   length = etohs (MB->header.length);

   if (length > (ESC_MBX0_sml - ESC_MBXHSIZE))
   {
      length = ESC_MBX0_sml - ESC_MBXHSIZE;
   }
   ESC_read (ESC_MBX0_sma + ESC_MBXHSIZE, MB->b, length);
   if (length + ESC_MBXHSIZE < ESC_MBX0_sml)
   {
      ESC_read (ESC_MBX0_sme, &length, 1);
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
   _MBXh *MBh = (_MBXh *)&MBX[n * ESC_MBXSIZE];
   uint8_t dummy = 0;
   uint16_t length;
   length = etohs (MBh->length);

   if (length > (ESC_MBX1_sml - ESC_MBXHSIZE))
   {
      length = ESC_MBX1_sml - ESC_MBXHSIZE;
   }
   ESC_write (ESC_MBX1_sma, MBh, ESC_MBXHSIZE + length);
   if (length + ESC_MBXHSIZE < ESC_MBX1_sml)
   {
      ESC_write (ESC_MBX1_sme, &dummy, 1);
   }

   ESCvar.mbxfree = 0;
}

/** TBD
 */
void ESC_ackmbxread (void)
{
   uint8_t dummy = 0;

   ESC_write (ESC_MBX1_sma, &dummy, 1);
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
   _MBXh *MBh;
   uint8_t n = ESC_MBXBUFFERS - 1;
   while ((n > 0) && (MBXcontrol[n].state))
   {
      n--;
   }
   if (n)
   {
      MBXcontrol[n].state = MBXstate_outclaim;
      MBh = (_MBXh *)&MBX[n * ESC_MBXSIZE];
      ESCvar.mbxcnt++;
      ESCvar.mbxcnt = (ESCvar.mbxcnt & 0x07);
      if (ESCvar.mbxcnt == 0)
      {
         ESCvar.mbxcnt = 1;
      }
      MBh->address = htoes (0x0000);      // destination is master
      MBh->channel = 0;
      MBh->priority = 0;
      MBh->mbxcnt = ESCvar.mbxcnt;
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
   uint8_t n = ESC_MBXBUFFERS - 1;
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
      mbxerr = (_MBXerr *) &MBX[MBXout * ESC_MBXSIZE];
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
   _MBXh *MBh = (_MBXh *)&MBX[0];

   if (ESCvar.MBXrun == 0)
   {
      /* nothing to do */
      return 0;
   }

   /* SM0/1 access */
   if (ESCvar.ALevent & (ESCREG_ALEVENT_SM0 | ESCREG_ALEVENT_SM1))
   {
      ESC_SMstatus (0);
      ESC_SMstatus (1);
   }

   /* outmbx read by master */
   if (ESCvar.mbxoutpost && (ESCvar.ALevent & ESCREG_ALEVENT_SM1))
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
         if (ESCvar.mbxoutpost == 0)
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
         /* Refresh SM status */
         ESC_SMstatus (1);
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
   if ((ESCvar.SM[0].MBXstat != 0) && (MBXcontrol[0].state == 0)
         && (ESCvar.mbxoutpost == 0) && (ESCvar.xoe == 0))
   {
      ESC_readmbx ();
      ESCvar.SM[0].MBXstat = 0;
      if (etohs (MBh->length) == 0)
      {
         MBX_error (MBXERR_INVALIDHEADER);
         /* drop mailbox */
         MBXcontrol[0].state = MBXstate_idle;
      }
      if ((MBh->mbxcnt != 0) && (MBh->mbxcnt == ESCvar.mbxincnt))
      {
         /* drop mailbox */
         MBXcontrol[0].state = MBXstate_idle;
      }
      ESCvar.mbxincnt = MBh->mbxcnt;
      return 1;
   }

   return 0;
}
/** Handler for incorrect or unsupported mailbox data. Write error response
 * in Mailbox.
 */
void ESC_xoeprocess (void)
{
   _MBXh *mbh;
   if (ESCvar.MBXrun == 0)
   {
      return;
   }
   if ((ESCvar.xoe == 0) && (MBXcontrol[0].state == MBXstate_inclaim))
   {
      mbh = (_MBXh *) &MBX[0];
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
   SM = (_ESCsm2 *) & ESCvar.SM[2];
   if ((etohs (SM->PSA) != ESC_SM2_sma) || (etohs (SM->Length) != ESCvar.ESC_SM2_sml)
       || (SM->Command != ESC_SM2_smc) || !(SM->ActESC & ESC_SM2_act))
   {
      ESCvar.SMtestresult = SMRESULT_ERRSM2;
      /* fail state change */
      return (ESCpreop | ESCerror);
   }
   if ((ESC_SM2_sma + (etohs (SM->Length) * 3)) > ESC_SM3_sma)
   {
      ESCvar.SMtestresult = SMRESULT_ERRSM2;
      /* SM2 overlaps SM3, fail state change */
      return (ESCpreop | ESCerror);
   }
   ESC_read (ESCREG_SM3, (void *) &ESCvar.SM[3], sizeof (ESCvar.SM[3]));
   SM = (_ESCsm2 *) & ESCvar.SM[3];
   if ((etohs (SM->PSA) != ESC_SM3_sma) || (etohs (SM->Length) != ESCvar.ESC_SM3_sml)
       || (SM->Command != ESC_SM3_smc) || !(SM->ActESC & ESC_SM3_act))
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
      CC_ATOMIC_SET(ESCvar.App.state, APPSTATE_INPUT);
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

   /* Exit here if polling */
   if (ESCvar.use_interrupt == 0)
   {
      return state;
   }

   if (state != (ESCpreop | ESCerror))
   {
      uint16_t dc_check_result;
      dc_check_result = ESC_checkDC();
      if(dc_check_result > 0)
      {
         ESC_ALerror (dc_check_result);
         state = (ESCpreop | ESCerror);

         ESC_SMdisable (2);
         ESC_SMdisable (3);
         CC_ATOMIC_SET(ESCvar.App.state, APPSTATE_IDLE);
      }
      else
      {
         if (ESCvar.esc_hw_interrupt_enable != NULL)
         {
            if(ESCvar.dcsync > 0)
            {
               ESCvar.esc_hw_interrupt_enable(ESCREG_ALEVENT_DC_SYNC0 |
                     ESCREG_ALEVENT_SM2);
            }
            else
            {
               ESCvar.esc_hw_interrupt_enable(ESCREG_ALEVENT_SM2);
            }
         }
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
   CC_ATOMIC_SET(ESCvar.App.state, APPSTATE_IDLE);
   ESC_SMdisable (3);
   ESC_SMdisable (2);

   /* Call interrupt disable hook case it have been configured  */
   if ((ESCvar.use_interrupt != 0) &&
         (ESCvar.esc_hw_interrupt_disable != NULL))
   {
      ESCvar.esc_hw_interrupt_disable (ESCREG_ALEVENT_DC_SYNC0 |
            ESCREG_ALEVENT_SM2);
   }
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
   CC_ATOMIC_OR(ESCvar.App.state, APPSTATE_OUTPUT);
   return state;

}

/** Unconditional stop of updating outputs by disabling Sync Manager 2.
 * Set the App.state to APPSTATE_INPUT. Call application hook APP_safeoutput
 * letting the user to set safe state values on outputs.
 *
 */
void ESC_stopoutput (void)
{
   CC_ATOMIC_AND(ESCvar.App.state, APPSTATE_INPUT);
   ESC_SMdisable (2);
   APP_safeoutput ();
}

/** The state handler acting on SyncManager Activation BIT(4)
 * events in the Al Event Request register 0x220.
 *
 */
void ESC_sm_act_event (void)
{
   uint8_t ac, an, as, ax, ax23;

   /* Have at least on Sync Manager  changed */
   if ((ESCvar.ALevent & ESCREG_ALEVENT_SMCHANGE) == 0)
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
   if ((as & ESCREG_AL_ALLBUTINITMASK) &&
       ((as == ESCboot) == 0) && ESCvar.MBXrun)
   {
      /* Validate Sync Managers, reading the Activation register will
       * acknowledge the SyncManager Activation event making us enter
       * this execution path.
       */
      ax = ESC_checkmbx (as);
      ax23 = ESC_checkSM23 (as);
      if ((an & ESCerror) && ((ac & ESCerror) == 0))
      {
         /* if in error then stay there */
      }
      /* Have we been forced to step down to INIT we will stop mailboxes,
       * update AL Status Code and exit ESC_state
       */
      else if (ax == (ESCinit | ESCerror))
      {
         /* If we have activated Inputs and Outputs we need to disable them */
         if (CC_ATOMIC_GET(ESCvar.App.state))
         {
            ESC_stopoutput ();
            ESC_stopinput ();
         }
         /* Stop mailboxes and update ALStatus code */
         ESC_stopmbx ();
         ESC_ALerror (ALERR_INVALIDMBXCONFIG);
         ESCvar.MBXrun = 0;
         ESC_ALstatus (ax);
         return;
      }
      /* Have we been forced to step down to PREOP we will stop inputs
       * and outputs, update AL Status Code and exit ESC_state
       */
      else if (CC_ATOMIC_GET(ESCvar.App.state) && (ax23 == (ESCpreop | ESCerror)))
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
      }
   }
   else
   {
      ESC_SMack (0);
      ESC_SMack (1);
      ESC_SMack (2);
      ESC_SMack (3);
      ESC_SMack (4);
      ESC_SMack (5);
      ESC_SMack (6);
      ESC_SMack (7);
   }
}
/** The state handler acting on ALControl Bit(0)
 * events in the Al Event Request register 0x220.
 *
 */
void ESC_state (void)
{
   uint8_t ac, an, as;

   /* Do we have a state change request pending */
   if (ESCvar.ALevent & ESCREG_ALEVENT_CONTROL)
   {
      ESC_read (ESCREG_ALCONTROL, (void *) &ESCvar.ALcontrol,
                sizeof (ESCvar.ALcontrol));
      ESCvar.ALcontrol = etohs (ESCvar.ALcontrol);
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

   /* Error state not acked, leave original */
   if ((an & ESCerror) && ((ac & ESCerror) == 0))
   {
      return;
   }

   /* Mask high bits ALcommand, low bits ALstatus */
   as = (ac << 4) | (as & 0x0f);

   /* Call post state change hook case it have been configured  */
   if (ESCvar.pre_state_change_hook != NULL)
   {
      ESCvar.pre_state_change_hook (&as, &an);
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
         COE_initDefaultValues ();
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
         ESC_stopinput ();
         ESC_stopmbx ();
         an = ESCinit;
         break;
      }
      case SAFEOP_TO_INIT:
      {
         ESC_stopinput ();
         ESC_stopmbx ();
         an = ESCinit;
         break;
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
         ESCvar.ESC_SM2_sml = sizeOfPDO (RX_PDO_OBJIDX, &ESCvar.sm2mappings,
                                         SMmap2, MAX_MAPPINGS_SM2);
         if (ESCvar.sm2mappings < 0)
         {
            an = ESCpreop | ESCerror;
            ESC_ALerror (ALERR_INVALIDOUTPUTSM);
            break;
         }

         ESCvar.ESC_SM3_sml = sizeOfPDO (TX_PDO_OBJIDX, &ESCvar.sm3mappings,
                                         SMmap3, MAX_MAPPINGS_SM3);
         if (ESCvar.sm3mappings < 0)
         {
            an = ESCpreop | ESCerror;
            ESC_ALerror (ALERR_INVALIDINPUTSM);
            break;
         }

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
         ESC_stopinput ();
         an = ESCpreop;
         break;
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
   if (ESCvar.post_state_change_hook != NULL)
   {
      ESCvar.post_state_change_hook (&as, &an);
   }

   if (!(an & ESCerror) && (ESCvar.ALerror))
   {
      /* clear error */
      ESC_ALerror (ALERR_NONE);
   }

   ESC_ALstatus (an);
   DPRINT ("state %x\n", an);
}
/** Function copying the application configuration variable
 * data to the stack local variable.
 *
 * @param[in] cfg   = Pointer to the Application configuration variable
 * holding application specific details. Data is copied.
 */
void ESC_config (esc_cfg_t * cfg)
{
   static sm_cfg_t mb0 = {MBX0_sma, MBX0_sml, MBX0_sme, MBX0_smc, 0};
   static sm_cfg_t mb1 = {MBX1_sma, MBX1_sml, MBX1_sme, MBX1_smc, 0};
   static sm_cfg_t mbboot0 = {MBX0_sma_b, MBX0_sml_b, MBX0_sme_b, MBX0_smc_b, 0};
   static sm_cfg_t mbboot1 = {MBX1_sma_b, MBX1_sml_b, MBX1_sme_b, MBX1_smc_b, 0};

   /* Configure stack */
   ESCvar.use_interrupt = cfg->use_interrupt;
   ESCvar.watchdogcnt = cfg->watchdog_cnt;

   ESCvar.mb[0] = mb0;
   ESCvar.mb[1] = mb1;
   ESCvar.mbboot[0] = mbboot0;
   ESCvar.mbboot[1] = mbboot1;

   ESCvar.set_defaults_hook = cfg->set_defaults_hook;
   ESCvar.pre_state_change_hook = cfg->pre_state_change_hook;
   ESCvar.post_state_change_hook = cfg->post_state_change_hook;
   ESCvar.application_hook = cfg->application_hook;
   ESCvar.safeoutput_override = cfg->safeoutput_override;
   ESCvar.pre_object_download_hook = cfg->pre_object_download_hook;
   ESCvar.post_object_download_hook = cfg->post_object_download_hook;
   ESCvar.rxpdo_override = cfg->rxpdo_override;
   ESCvar.txpdo_override = cfg->txpdo_override;
   ESCvar.esc_hw_interrupt_enable = cfg->esc_hw_interrupt_enable;
   ESCvar.esc_hw_interrupt_disable = cfg->esc_hw_interrupt_disable;
   ESCvar.esc_hw_eep_handler = cfg->esc_hw_eep_handler;
   ESCvar.esc_check_dc_handler = cfg->esc_check_dc_handler;
}
