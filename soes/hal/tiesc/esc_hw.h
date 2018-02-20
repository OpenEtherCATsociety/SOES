/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * ESC hardware specifoc EEPROM emulation functions.
 */

#ifndef __esc_hw__
#define __esc_hw__

#include <cc.h>

void EEP_hw_process (void);
void ESC_eep_handler(void);
void ESC_interrupt_enable (uint32_t mask);
void ESC_interrupt_disable (uint32_t mask);

#endif

