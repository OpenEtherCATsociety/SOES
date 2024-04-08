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
static uint16_t eep_read_size = 8U;
static uint16_t (*eep_reload_ptr)(void) = NULL;

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
            /* handle read request */
            if (EEP_read (stat.addr * 2U /* sizeof(uint16_t) */, eep_buf, eep_read_size) != 0) {
               stat.contstat.bits.ackErr = 1;
            }
            else {
               ESC_write(ESCREG_EEDATA, eep_buf, eep_read_size);
            }
            break;

         case EEP_CMD_RELOAD:
            /* user defined reload if set */
            if (eep_reload_ptr != NULL) {
               uint16_t reload_ret = (*eep_reload_ptr)();
               if (reload_ret != 0) {
                  stat.contstat.bits.ackErr = 1;
                  if (reload_ret & EEP_ERROR_CSUM) {
                     stat.contstat.bits.csumErr = 1;
                  }
               }
            }
            else {
                if (eep_read_size == 8U) {
                   /* handle reload request */
                   if (EEP_read(stat.addr * 2U /* sizeof(uint16_t) */, eep_buf, eep_read_size) != 0) {
                      stat.contstat.bits.ackErr = 1;
                   }
                   else {
                      ESC_write(ESCREG_EEDATA, eep_buf, eep_read_size);
                   }
                }
                else {
                  /* Default handler of reload request for 4 Byte read, load config alias.
                   * To support other ESC behavior, implement user defined reload.
                   */
                  if (EEP_read(EEP_CONFIG_ALIAS_WORD_OFFSET * 2U /* sizeof(uint16_t) */,
                        eep_buf,
                        2U /* 2 Bytes config alias*/) != 0) {
                     stat.contstat.bits.ackErr = 1;
                  }
                  else {
                     ESC_write(ESCREG_EEDATA, eep_buf, 2U /* 2 Bytes config alias*/);
                  }
               }
            }
            break;

         case EEP_CMD_WRITE:
            /* handle write request */
            ESC_read (ESCREG_EEDATA, eep_buf, EEP_WRITE_SIZE);
            if (EEP_write (stat.addr * 2U /* sizeof(uint16_t) */, eep_buf, EEP_WRITE_SIZE) != 0) {
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

/** EPP Set read size, 4 Byte or 8 Byte depending on ESC.
 *  Default 8 Byte.
 */
void EEP_set_read_size (uint16_t read_size)
{
   if ((read_size == 8U) || (read_size == 4U))
   {
      eep_read_size = read_size;
   }
}

/** EPP Set reload fucntion pointer.
 *  Function shall return 0 on success, else return
 *  defined error mask eg. EEP_ERROR_CSUM on CRC error
 *  on reload.
 */
void EEP_set_reload_function_pointer (uint16_t (*reload_ptr)(void))
{
   eep_reload_ptr = reload_ptr;
}
