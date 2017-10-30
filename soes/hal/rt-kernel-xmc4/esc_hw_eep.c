/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * ESC hardware specific EEPROM emulation functions.
 */

#include <cc.h>
#include <fce.h>
#include <string.h>
#include <drivers/nor/nor.h>
#include "esc.h"
#include "esc_hw_eep.h"

extern const uint8_t _binary_sii_eeprom_bin_start;
extern const uint8_t _binary_sii_eeprom_bin_end;

#define SII_EE_DEFLT (&_binary_sii_eeprom_bin_start)
#define SII_EE_DEFLT_SIZE (uint32_t)(&_binary_sii_eeprom_bin_end - &_binary_sii_eeprom_bin_start)

#define EEP_DEFAULT_BTN_INIT()   { }
#define EEP_DEFAULT_BTN_STATE()  0

#define EEP_BUSY_LED_INIT()      { }
#define EEP_BUSY_LED_ON()        { }
#define EEP_BUSY_LED_OFF()       { }

#if EEP_BYTES_PER_BLOCK > EEP_BYTES_PER_SECTOR
#error EEP_BYTES_PER_BLOCK needs to fit into EEP_BYTES_PER_SECTOR
#endif

/** CRC engine configuration */
static const fce_kernel_cfg_t fce_config =
{
   .crc_kernel_addr = FCE_KE0_BASE,
   .kernel_cfg = 0,
   .seed = 0xffffffff
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

static drv_t * drv;

static void init_flash_data(void);
static void find_latest_block(eep_block_t *addr);
static eep_block_t *get_next_block(eep_block_t *block);
static eep_block_t *cleanup_unused_sect(eep_block_t *block);

static int32_t is_sector_empty(uint32_t *addr);


/** Initialize EEPROM emulation (load default data, validate checksums, ...).
 *
 */
void EEP_init (void)
{
   /* initialize write buffer */
   memset(&eep_write_buf, 0, EEP_BYTES_PER_BLOCK);

   /* Initialize the FCE Configuration */
   fce_init(&fce_config);

   drv = dev_find_driver ("/pflash");
   ASSERT (drv != NULL);

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
      nor_write(drv, EEP_FLASH_SECTOR_OFFSET((uint32_t)eep_write_dst),
            EEP_BYTES_PER_PAGE, (const uint8_t *)eep_write_src);
      eep_write_src += (EEP_BYTES_PER_PAGE / sizeof(*eep_write_src));
      eep_write_dst += (EEP_BYTES_PER_PAGE / sizeof(*eep_write_dst));

      /* update counter */
      eep_write_page++;

      /* check for finished job */
      if (eep_write_page >= EEP_PAGES_PER_BLOCK) {
         /* update block pointer and reset write state */
         eep_curr_block = eep_next_block;
         eep_next_block = NULL;
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
      eep_write_buf.header.crc = fce_crc32 (&fce_config,
            (const uint32_t *)eep_write_buf.data, EEP_DATA_BYTES);

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
   uint32_t dst;

   /* erase both sectors */
   nor_erase(drv, EEP_FLASH_SECTOR_OFFSET(EEP_SECTOR_A), EEP_BYTES_PER_SECTOR);
   nor_erase(drv, EEP_FLASH_SECTOR_OFFSET(EEP_SECTOR_B), EEP_BYTES_PER_SECTOR);

   /* copy default data to write buffer */

   memcpy(eep_write_buf.data, SII_EE_DEFLT, (SII_EE_DEFLT_SIZE < EEP_EMU_BYTES) ? SII_EE_DEFLT_SIZE : EEP_EMU_BYTES);
   /*
    * memcpy(eep_write_buf.data, SII_EE_DEFLT, (SII_EE_DEFLT_SIZE < EEP_EMU_BYTES) ? SII_EE_DEFLT_SIZE : EEP_EMU_BYTES);
    */
   /* setup header data */
   eep_write_buf.header.seq = 0;
   eep_write_buf.header.crc = fce_crc32 (&fce_config,
         (const uint32_t *)eep_write_buf.data, EEP_DATA_BYTES);

   /* write pages */
   src = (const uint32_t *) &eep_write_buf;
   dst = EEP_SECTOR_A;
   for (i = 0; i < EEP_PAGES_PER_BLOCK; i++) {

      nor_write(drv, EEP_FLASH_SECTOR_OFFSET(dst), EEP_BYTES_PER_PAGE,
            (const uint8_t *)src);

      src += (EEP_BYTES_PER_PAGE / sizeof(*src));
      dst += EEP_BYTES_PER_PAGE;
   }

   /* set current block */
   eep_curr_block = (eep_block_t *) EEP_SECTOR_A;
}

static void find_latest_block(eep_block_t *addr)
{
   uint32_t blk, crc;

   for (blk = 0; blk < EPP_BLOCKS_PER_SECT; blk++, addr++) {
      /* check crc, skip invalid blocks */
      crc = fce_crc32 (&fce_config, (const uint32_t *) addr->data,
            EEP_DATA_BYTES);
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
   uint32_t sect_addr = (((uint32_t)block) & ~(EEP_BYTES_PER_SECTOR - 1));

   if (sect_addr == EEP_SECTOR_A) {
      sect_addr = EEP_SECTOR_B;
   } else {
      sect_addr = EEP_SECTOR_A;
   }

   /* check if sector is empty, erase if not */
   if (!is_sector_empty((uint32_t *)sect_addr)) {
      nor_erase(drv, EEP_FLASH_SECTOR_OFFSET(sect_addr), EEP_BYTES_PER_SECTOR);
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


