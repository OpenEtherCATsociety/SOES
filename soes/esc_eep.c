/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
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

