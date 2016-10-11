/*
 * SOES Simple Open EtherCAT Slave
 *
 * Copyright (C) 2010 ZBE Inc.
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
