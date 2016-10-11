/*
 * SOES Simple Open EtherCAT Slave
 *
 * File    : esc_hw_eep.h
 * Version : 1.0.0
 * Date    : 26-08-2016
 * Copyright (C) 2016 Sascha Ittner
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
 * ESC hardware specifoc EEPROM emulation functions.
 */

#ifndef __esc_hw_eep__
#define __esc_hw_eep__

#include "cc.h"
#include "esc_eep.h"

#include "xmc_gpio.h"
#include "xmc_flash.h"
#include "xmc_fce.h"

/* if defined: reload default EEPROM content if pulled low */
#define EEP_DEFAULT_BTN		P3_4

/* if defined: flash BUSY indicator */
#define EEP_BUSY_LED		P4_0

/* used CRC32 kernel for checksum calculation */
#define EPP_FCE_CRC32		XMC_FCE_CRC32_0

/* idle timeout in ns before actual flash write will be issued */
#define EEP_IDLE_TIMEOUT	100000000

/* Packes per emulated EEPROM block*/
#define EEP_PAGES_PER_BLOCK	16

/* flash sector select */
#define EEP_BYTES_PER_SECTOR	0x04000
#define EEP_SECTOR_A		XMC_FLASH_SECTOR_6
#define EEP_SECTOR_B		XMC_FLASH_SECTOR_7

/* block header */
typedef struct CC_PACKED
{
   int32_t seq;
   uint32_t crc;
} eep_header_t;

/* calulate resulting sizes */
#define EEP_BYTES_PER_BLOCK	(EEP_PAGES_PER_BLOCK * XMC_FLASH_BYTES_PER_PAGE)
#define EPP_BLOCKS_PER_SECT	(EEP_BYTES_PER_SECTOR / EEP_BYTES_PER_BLOCK)
#define EEP_DATA_BYTES		(EEP_BYTES_PER_BLOCK - sizeof(eep_header_t))

/* eeprom size increments in steps of 0x80 bytes */
#define EEP_EMU_BYTES		(EEP_DATA_BYTES & ~0x7f)

/* block structure */
typedef struct CC_PACKED
{
   eep_header_t header;
   uint8_t data[EEP_DATA_BYTES];
} eep_block_t;

/* periodic task */
void EEP_hw_process (void);

#endif

