/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * ESC hardware specific EEPROM emulation functions.
 */

#include <cc.h>
#include <string.h>
#include "esc.h"
#include "esc_hw_eep.h"
#include "tieschw.h"

extern const uint8_t _binary_sii_eeprom_bin_start;
const unsigned char * tiesc_eeprom = &_binary_sii_eeprom_bin_start;
extern uint8_t eeprom_cache[TIESC_EEPROM_SIZE];

/** Initialize EEPROM emulation (load default data, validate checksums, ...).
 *
 */
void EEP_init (void)
{
}

/** EEPROM emulation controller side periodic task.
 *
 */
void EEP_hw_process (void)
{
  return;
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
   if (addr >= TIESC_EEPROM_SIZE) {
      return 1;
   }

   /* read data from ram buffer */
   memcpy(data, eeprom_cache + addr, count);

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

   /* write data to ram buffer */
   memcpy(eeprom_cache + addr, data, count);

   return 0;
}


