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

#include <bsp.h>
#include <cc.h>
#include "esc_eep.h"

/* idle timeout in ns before actual flash write will be issued */
#define EEP_IDLE_TIMEOUT	100000000

/* Pages per emulated EEPROM block */
#define EEP_BYTES_PER_SECTOR  XMC4_EEPROM_SECTOR_SIZE_BYTES
#define EEP_BYTES_PER_PAGE    XMC4_PAGE_SIZE_BYTES
#define EEP_PAGES_PER_BLOCK   16

/* block header */
typedef struct CC_PACKED
{
   int32_t seq;
   uint32_t crc;
} eep_header_t;

/* calculate resulting sizes */
#define EEP_BYTES_PER_BLOCK	(EEP_PAGES_PER_BLOCK * EEP_BYTES_PER_PAGE)
#define EPP_BLOCKS_PER_SECT	(EEP_BYTES_PER_SECTOR / EEP_BYTES_PER_BLOCK)
#define EEP_DATA_BYTES		   (EEP_BYTES_PER_BLOCK - sizeof(eep_header_t))

/* eeprom size increments in steps of 0x80 bytes */
#define EEP_EMU_BYTES         (EEP_DATA_BYTES & ~0x7f)

extern uint8_t eep_write_pending;

/* block structure */
typedef struct CC_PACKED
{
   eep_header_t header;
   uint8_t data[EEP_DATA_BYTES];
} eep_block_t;

/* periodic task */
void EEP_hw_process (void);

#endif


