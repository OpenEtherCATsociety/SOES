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

#include <cc.h>
#include "esc.h"

/* EEPROM commands */
#define EEP_CMD_IDLE        0x0
#define EEP_CMD_READ        0x1
#define EEP_CMD_WRITE       0x2
#define EEP_CMD_RELOAD      0x3

/* read/write size */
#define EEP_READ_SIZE       8
#define EEP_WRITE_SIZE      2

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

/**
 * ECAT EEPROM configuration area data structure
 */
typedef union eep_config
{
  struct
  {
    uint16_t pdi_control;
    uint16_t pdi_configuration;
    uint16_t sync_impulse_len;
    uint16_t pdi_configuration2;
    uint16_t configured_station_alias;
    uint8_t  reserved[4];
    uint16_t checksum;
  };
  uint32_t dword[4]; /**< Four 32 bit double word equivalent to 8 16 bit configuration area word. */
}eep_config_t;

/* periodic task */
void EEP_process (void);

/* From hardware file */
void EEP_init (void);
int8_t EEP_read (uint32_t addr, uint8_t *data, uint16_t size);
int8_t EEP_write (uint32_t addr, uint8_t *data, uint16_t size);

#endif
