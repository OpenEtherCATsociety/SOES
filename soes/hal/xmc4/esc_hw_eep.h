/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * ESC hardware specifoc EEPROM emulation functions.
 */

#ifndef __esc_hw_eep__
#define __esc_hw_eep__

#include "esc_hw.h"
#include "cc.h"
#include "xmc_gpio.h"
#include "xmc_flash.h"
#include "xmc_fce.h"

/* if defined: reload default EEPROM content if pulled low */

#ifdef XMC4800_F144x2048
#define EEP_DEFAULT_BTN    P15_13
/* if defined: flash BUSY indicator */
#define EEP_BUSY_LED       P5_9
#endif


#ifdef XMC4300_F100x256
#define EEP_DEFAULT_BTN     P3_4
/* if defined: flash BUSY indicator */
#define EEP_BUSY_LED        P4_0
#endif

/* used CRC32 kernel for checksum calculation */
#define EPP_FCE_CRC32       XMC_FCE_CRC32_0

/* idle timeout in ns before actual flash write will be issued */
#define EEP_IDLE_TIMEOUT    100000000

/* Packes per emulated EEPROM block*/
#define EEP_PAGES_PER_BLOCK 16

#ifdef XMC4800_F144x2048
#define EEP_BYTES_PER_SECTOR    0x040000
#define EEP_SECTOR_A            XMC_FLASH_SECTOR_14
#define EEP_SECTOR_B            XMC_FLASH_SECTOR_15
#endif

#ifdef XMC4300_F100x256
#define EEP_BYTES_PER_SECTOR    0x04000
#define EEP_SECTOR_A            XMC_FLASH_SECTOR_6
#define EEP_SECTOR_B            XMC_FLASH_SECTOR_7
#endif
/* block header */
typedef struct CC_PACKED
{
   int32_t seq;
   uint32_t crc;
} eep_header_t;

/* calulate resulting sizes */
#define EEP_BYTES_PER_BLOCK (EEP_PAGES_PER_BLOCK * XMC_FLASH_BYTES_PER_PAGE)
#define EPP_BLOCKS_PER_SECT (EEP_BYTES_PER_SECTOR / EEP_BYTES_PER_BLOCK)
#define EEP_DATA_BYTES      (EEP_BYTES_PER_BLOCK - sizeof(eep_header_t))

/* eeprom size increments in steps of 0x80 bytes */
#define EEP_EMU_BYTES       (EEP_DATA_BYTES & ~0x7f)

/* block structure */
typedef struct CC_PACKED
{
   eep_header_t header;
   uint8_t data[EEP_DATA_BYTES];
} eep_block_t;

/* periodic task */
void EEP_hw_process (void);

#endif

