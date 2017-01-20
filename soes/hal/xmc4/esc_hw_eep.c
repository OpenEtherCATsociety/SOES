/*
 * SOES Simple Open EtherCAT Slave
 *
 * File    : esc_hw_eep.c
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
 * ESC hardware specific EEPROM emulation functions.
 */

#include "cc.h"
#include "esc.h"
#include "esc_hw_eep.h"

#include <string.h>

extern const uint8_t _binary_sii_eeprom_bin_start;
extern const uint8_t _binary_sii_eeprom_bin_end;

#define SII_EE_DEFLT (&_binary_sii_eeprom_bin_start)
#define SII_EE_DEFLT_SIZE (uint32_t)(&_binary_sii_eeprom_bin_end - &_binary_sii_eeprom_bin_start)

#if EEP_BYTES_PER_BLOCK > EEP_BYTES_PER_SECTOR
#error EEP_BYTES_PER_BLOCK needs to fit into EEP_BYTES_PER_SECTOR
#endif

static const XMC_FCE_t fce_config =
{
   .kernel_ptr = EPP_FCE_CRC32,
   .fce_cfg_update.config_refin = XMC_FCE_REFIN_RESET,
   .fce_cfg_update.config_refout = XMC_FCE_REFOUT_RESET,
   .fce_cfg_update.config_xsel = XMC_FCE_INVSEL_RESET,
   .seedvalue = 0
};

static uint8_t eep_buf[EEP_EMU_BYTES];
static uint8_t eep_buf_dirty;
static uint32_t eep_last_write;

static uint8_t eep_write_req;
static eep_block_t *eep_curr_block;
static eep_block_t *eep_next_block;

static uint32_t *eep_write_src;
static uint32_t *eep_write_dst;
static uint32_t eep_write_page;

static eep_block_t eep_write_buf;

static void init_flash_data(void);
static void find_latest_block(eep_block_t *addr);
static eep_block_t *get_next_block(eep_block_t *block);
static eep_block_t *cleanup_unused_sect(eep_block_t *block);

static int32_t is_sector_empty(uint32_t *addr);
static uint32_t crc32(const uint8_t *data, uint32_t length);

#ifdef EEP_DEFAULT_BTN

static const XMC_GPIO_CONFIG_t gpio_config_btn = {
  .mode = XMC_GPIO_MODE_INPUT_INVERTED_PULL_UP,
  .output_level = 0,
  .output_strength = 0
};

#define EEP_DEFAULT_BTN_INIT()  XMC_GPIO_Init(EEP_DEFAULT_BTN, &gpio_config_btn)
#define EEP_DEFAULT_BTN_STATE() XMC_GPIO_GetInput(EEP_DEFAULT_BTN)

#else

#define EEP_DEFAULT_BTN_INIT()  { }
#define EEP_DEFAULT_BTN_STATE() 0

#endif

#ifdef EEP_BUSY_LED

static const XMC_GPIO_CONFIG_t gpio_config_led = {
  .mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
  .output_level = XMC_GPIO_OUTPUT_LEVEL_LOW,
  .output_strength = XMC_GPIO_OUTPUT_STRENGTH_STRONG_SOFT_EDGE
};

#define EEP_BUSY_LED_INIT() XMC_GPIO_Init(EEP_BUSY_LED, &gpio_config_led)
#define EEP_BUSY_LED_ON()   XMC_GPIO_SetOutputHigh(EEP_BUSY_LED)
#define EEP_BUSY_LED_OFF()  XMC_GPIO_SetOutputLow(EEP_BUSY_LED)

#else

#define EEP_BUSY_LED_INIT() { }
#define EEP_BUSY_LED_ON()   { }
#define EEP_BUSY_LED_OFF()  { }

#endif

/** Initialize EEPROM emulation (load default data, validate checksums, ...).
 *
 */
void EEP_init (void)
{
   /* initialize write buffer */
   memset(&eep_write_buf, 0, EEP_BYTES_PER_BLOCK);

   /* configure I/Os */
   EEP_DEFAULT_BTN_INIT();
   EEP_BUSY_LED_INIT();

   /* Enable FCE module */
   XMC_FCE_Enable();

   /* Initialize the FCE Configuration */
   XMC_FCE_Init(&fce_config);

   /* try to find latest block in both sectors */
   eep_curr_block = NULL;
   if (!EEP_DEFAULT_BTN_STATE()) {
      find_latest_block((eep_block_t *) EEP_SECTOR_A);
      find_latest_block((eep_block_t *) EEP_SECTOR_B);
   }

   EEP_BUSY_LED_ON();

   /* no valid block found -> initialize flash with default data */
   if (eep_curr_block == NULL) {
      init_flash_data();
   }

   /* cleanup unused block */
   cleanup_unused_sect(eep_curr_block);

   /* copy data from block to emu buffer */
   memcpy(eep_buf, eep_curr_block->data, EEP_EMU_BYTES);

   EEP_BUSY_LED_OFF();

   /* initialize state variables */
   eep_buf_dirty = 0;
   eep_last_write = 0;
   eep_write_req = 0;
   eep_next_block = NULL;
}

/** EEPROM emulation controller side periodic task.
 *
 */
void EEP_hw_process (void)
{
   /* check for dirty buffer and set write */
   if (eep_buf_dirty) {
      int32_t idle_time = ((int32_t) ESCvar.Time) - ((int32_t) eep_last_write);
      if (idle_time > EEP_IDLE_TIMEOUT) {
         eep_buf_dirty = 0;
         eep_write_req = 1;
      }
   }

   /* check for write process */
   if (eep_next_block != NULL) {
      /* write flash page */
      XMC_FLASH_ProgramPage(eep_write_dst, eep_write_src);
      eep_write_src += XMC_FLASH_WORDS_PER_PAGE;
      eep_write_dst += XMC_FLASH_WORDS_PER_PAGE;

      /* update counter */
      eep_write_page++;

      /* check for finished job */
      if (eep_write_page >= EEP_PAGES_PER_BLOCK) {
         /* update block pointer and reset write state */
         eep_curr_block = eep_next_block;
         eep_next_block = NULL;

         EEP_BUSY_LED_OFF();
      }

      return;
   }

   /* start write of new block */
   if (eep_write_req) {
      EEP_BUSY_LED_ON();

      /* get next block */
      eep_next_block = get_next_block(eep_curr_block);

      /* copy data */
      memcpy(eep_write_buf.data, eep_buf, EEP_EMU_BYTES);

      /* setup header */
      eep_write_buf.header.seq = eep_curr_block->header.seq + 1;
      eep_write_buf.header.crc = crc32 (eep_write_buf.data, EEP_DATA_BYTES);

      /* initialize write position */
      eep_write_src = (uint32_t *) &eep_write_buf;
      eep_write_dst = (uint32_t *) eep_next_block;
      eep_write_page = 0;

      /* reset write request */
      eep_write_req = 0;
   }
}

/** EEPROM read function
 *
 * @param[in]   addr     = EEPROM byte address
 * @param[out]  data     = pointer to buffer of output data
 * @param[in]   count    = number of bytes to read
 * @return 0 on OK, 1 on error
 */
int8_t EEP_read (uint32_t addr, uint8_t *data, uint16_t count)
{
   if (addr >= EEP_EMU_BYTES) {
      return 1;
   }

   /* read data from ram buffer */
   memcpy(data, eep_buf + addr, count);

   return 0;
}

/** EEPROM write function
 *
 * @param[in]   addr     = EEPROM byte address
 * @param[out]  data     = pointer to buffer of input data
 * @param[in]   count    = number of bytes to write
 * @return 0 on OK, 1 on error
 */
int8_t EEP_write (uint32_t addr, uint8_t *data, uint16_t count)
{
   if (addr >= EEP_EMU_BYTES) {
      return 1;
   }

   /* write data to ram buffer */
   memcpy(eep_buf + addr, data, count);

   /* mark buffer as dirty */
   eep_buf_dirty = 1;
   eep_write_req = 0;
   eep_last_write = ESCvar.Time;

   return 0;
}

static void init_flash_data(void)
{
   uint32_t i;
   const uint32_t *src;
   uint32_t *dst;

   /* erase both sectors */
   XMC_FLASH_EraseSector(EEP_SECTOR_A);
   XMC_FLASH_EraseSector(EEP_SECTOR_B);

   /* copy default data to write buffer */
   memcpy(eep_write_buf.data, SII_EE_DEFLT, (SII_EE_DEFLT_SIZE < EEP_EMU_BYTES) ? SII_EE_DEFLT_SIZE : EEP_EMU_BYTES);

   /* setup header data */
   eep_write_buf.header.seq = 0;
   eep_write_buf.header.crc = crc32 (eep_write_buf.data, EEP_DATA_BYTES);

   /* write pages */
   src = (const uint32_t *) &eep_write_buf;
   dst = EEP_SECTOR_A;
   for (i = 0; i < EEP_PAGES_PER_BLOCK; i++) {
      XMC_FLASH_ProgramPage(dst, src);
      src += XMC_FLASH_WORDS_PER_PAGE;
      dst += XMC_FLASH_WORDS_PER_PAGE;
   }

   /* set current block */
   eep_curr_block = (eep_block_t *) EEP_SECTOR_A;
}

static void find_latest_block(eep_block_t *addr)
{
   uint32_t blk, crc;

   for (blk = 0; blk < EPP_BLOCKS_PER_SECT; blk++, addr++) {
      /* check crc, skip invalid blocks */
      crc = crc32 (addr->data, EEP_DATA_BYTES);
      if (addr->header.crc != crc) {
         continue;
      }

      /* check sequence number and update last pointer */
      if (eep_curr_block == NULL || (addr->header.seq - eep_curr_block->header.seq) > 0) {
         eep_curr_block = addr;
      }
   }
}

static eep_block_t *get_next_block(eep_block_t *block)
{
   /* simple case: new block fits in current sector */
   uint32_t sect_offset = ((uint32_t)block) & (EEP_BYTES_PER_SECTOR - 1);
   if ((sect_offset + EEP_BYTES_PER_BLOCK) < EEP_BYTES_PER_SECTOR) {
      return block + 1;
   }

   /* use other sector */
   return cleanup_unused_sect(block);
}

static eep_block_t *cleanup_unused_sect(eep_block_t *block)
{
   /* get other sector */
   uint32_t *sect_addr = (uint32_t *) (((uint32_t)block) & ~(EEP_BYTES_PER_SECTOR - 1));
   if (sect_addr == EEP_SECTOR_A) {
      sect_addr = EEP_SECTOR_B;
   } else {
      sect_addr = EEP_SECTOR_A;
   }

   /* check if sector is empty, erase if not */
   if (!is_sector_empty(sect_addr)) {
      XMC_FLASH_EraseSector(sect_addr);
   }

   return (eep_block_t *) sect_addr;
}

static int32_t is_sector_empty(uint32_t *addr)
{
   uint32_t i;

   /* check for all bytes erased */
   for (i=0; i<EEP_BYTES_PER_SECTOR; i+=sizeof(uint32_t), addr++) {
      if (*addr != 0) {
        return 0;
      }
   }
   return 1;
}

static uint32_t crc32(const uint8_t *data, uint32_t length)
{
   uint32_t crc;

   XMC_FCE_InitializeSeedValue(&fce_config, 0xffffffff);
   XMC_FCE_CalculateCRC32(&fce_config, (const uint32_t *) data, length & ~3L, &crc);
   XMC_FCE_GetCRCResult(&fce_config, &crc);

   return crc;
}

