/*
 * SOES Simple Open EtherCAT Slave
 *
 * Copyright (C) 2010 ZBE Inc.
 * Copyright (C) 2011-2013 Arthur Ketels.
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

 /** \file
 * \brief
 * ESI EEPROM emulator module.
 */

#include "cc.h"
#include "esc.h"
#include "esc_eep.h"

#include <string.h>

static uint8_t eep_buf[8];

/** EPP periodic task of ESC side EEPROM emulation.
 *
 */
void EEP_process (void)
{
   eep_stat_t stat;

   /* check for eeprom event */
   if ((ESCvar.ALevent & ESCREG_ALEVENT_EEP) == 0) {
     return;
   }

   while (1) {
      /* read eeprom status */
      ESC_read (ESCREG_EECONTSTAT, &stat, sizeof (eep_stat_t));
      stat.contstat.reg = etohs(stat.contstat.reg);
      stat.addr = etohl(stat.addr);

      /* check busy flag, exit if job finished */
      if (!stat.contstat.bits.busy) {
        return;
      }

      /* clear error bits */
      stat.contstat.bits.csumErr = 0;
      stat.contstat.bits.eeLoading = 0;
      stat.contstat.bits.ackErr = 0;
      stat.contstat.bits.wrErr = 0;

      /* process commands */
      switch (stat.contstat.bits.cmdReg) {
         case EEP_CMD_IDLE:
            break;

         case EEP_CMD_READ:
         case EEP_CMD_RELOAD:
            /* handle read request */
            if (EEP_read (stat.addr * sizeof(uint16_t), eep_buf, EEP_READ_SIZE) != 0) {
               stat.contstat.bits.ackErr = 1;
            } else {
               ESC_write (ESCREG_EEDATA, eep_buf, EEP_READ_SIZE);
            }
            break;

         case EEP_CMD_WRITE:
            /* handle write request */
            ESC_read (ESCREG_EEDATA, eep_buf, EEP_WRITE_SIZE);
            if (EEP_write (stat.addr * sizeof(uint16_t), eep_buf, EEP_WRITE_SIZE) != 0) {
               stat.contstat.bits.ackErr = 1;
            }
            break;

         default:
            stat.contstat.bits.ackErr = 1;
      }

      /* acknowledge command */
      stat.contstat.reg = htoes(stat.contstat.reg);
      ESC_write (ESCREG_EECONTSTAT, &stat.contstat.reg, sizeof(uint16_t));
   }
}

