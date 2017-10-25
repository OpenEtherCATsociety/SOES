/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Headerfile for esc_eep.c
 */

#ifndef __esc_eep__
#define __esc_eep__

#include "cc.h"

/* EEPROM emulation related ESC registers */
#define ESCREG_EECONTSTAT	0x0502
#define ESCREG_EEDATA		0x0508
#define ESCREG_ALEVENT_EEP	0x0020

/* EEPROM commands */
#define EEP_CMD_IDLE		0x0
#define EEP_CMD_READ		0x1
#define EEP_CMD_WRITE		0x2
#define EEP_CMD_RELOAD		0x3

/* read/write size */
#define EEP_READ_SIZE		8
#define EEP_WRITE_SIZE		2

/* CONSTAT register content */
typedef struct CC_PACKED
{
   union {
     uint16_t reg;
     struct {
        uint8_t wrEnable:1;
        uint8_t reserved:4;
        uint8_t eeEmulated:1;
        uint8_t eightByteRead:1;
        uint8_t twoByteAddr:1;

        uint8_t cmdReg:3;
        uint8_t csumErr:1;
        uint8_t eeLoading:1;
        uint8_t ackErr:1;
        uint8_t wrErr:1;
        uint8_t busy:1;
      } bits;
   } contstat;

   uint32_t addr;
} eep_stat_t;

/* periodic task */
void EEP_process (void);

/* From hardware file */
void EEP_init (void);
int8_t EEP_read (uint32_t addr, uint8_t *data, uint16_t size);
int8_t EEP_write (uint32_t addr, uint8_t *data, uint16_t size);

#endif
