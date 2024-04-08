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

/* EEPROM ERRORs */
#define EEP_ERROR_CSUM                  (1U << 11)
#define EEP_ERROR_LOADING               (1U << 12)
#define EEP_ERROR_ACK                   (1U << 13)
#define EEP_ERROR_WRITE                 (1U << 14)

/* EEPROM commands */
#define EEP_CMD_IDLE                    0x0
#define EEP_CMD_READ                    0x1
#define EEP_CMD_WRITE                   0x2
#define EEP_CMD_RELOAD                  0x4

/* write size */
#define EEP_WRITE_SIZE                  2

/* EEPROm word offset */
#define EEP_CONFIG_ALIAS_WORD_OFFSET    4

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

/**
 * Application Notes: EEPROM emulation
 *
 * NOTE: Special handling needed when 4 Byte read is supported.
 *
 * Ref. ET1100 Datasheet sec2_registers_3i0, chapter 2.45.1,
 * "EEPROM emulation with 32 bit EEPROM data register (0x0502[6]=0)".
 *
 * For a Reload command, fill the EEPROM Data register with the
 * values shown in the chapter 2.45.1 before acknowledging
 * the command. These values are automatically transferred to the
 * designated registers after the Reload command is acknowledged.
 *
 * NOTE: When 4 Byte read is supported, EEP_process will only load
 * config alias on reload.
 *
 * NOTE: EEP_process support implementing a custom reload function
 * for both 4 Byte and 8 Byte read support.
 *
 * NOTE: Code snippet for custom reload function when 4 Byte read is supported.
 *
 * uint16_t reload_ptr(void)
 * {
 *     eep_config_t ee_cfg;
 *
 *     // Read configuration area
 *     EEP_read(0, &ee_cfg, sizeof(ee_cfg);
 *
 *     // Check CRC
 *     if(is_crc_ok(&ee_cfg) == true)
 *     {
 *        // Write config alias to EEPROM data registers.
 *        // Will be loaded to 0x12:13 on command ack.
 *        ESC_write(ESCREG_EEDATA,
 *             &ee_cfg.configured_station_alias,
 *             sizeof(configured_station_alias));
 *        // Return 0 to indicate success
 *        return 0;
 *     }
 *     else
 *     {
 *        // CRC error mask
 *        return EEP_ERROR_CSUM;
 *     }
 * }
 * NOTE: Code snippet for custom reload function when 8 Byte read is supported.
 *
 * uint16_t reload_ptr(void)
 * {
 *     eep_config_t ee_cfg;
 *     eep_stat_t stat;
 *
 *     // Read configuration area
 *     EEP_read(0, &ee_cfg, sizeof(ee_cfg);
 *
 *     // Check CRC
 *     if(is_crc_ok(&ee_cfg) == true)
 *     {
 *         // read requested EEPROM address
 *         ESC_read (ESCREG_EECONTSTAT, &stat, sizeof (eep_stat_t));
 *         stat.addr = etohl(stat.addr);
 *         // Load EEPROM data at requested EEPROM address
 *         EEP_read (stat.addr * sizeof(uint16_t), eep_buf, 8U);
 *         // Write loaded data to EEPROM data registers
 *         ESC_write(ESCREG_EEDATA, eep_buf, 8U);
 *
 *        // Return 0 to indicate success
 *        return 0;
 *     }
 *     else
 *     {
 *        // CRC error mask
 *        return EEP_ERROR_CSUM;
 *     }
 * }
 */

/* Set eep internal variables */
void EEP_set_read_size (uint16_t read_size);
void EEP_set_reload_function_pointer (uint16_t (*reload_ptr)(void));

/* From hardware file */
void EEP_init (void);
int8_t EEP_read (uint32_t addr, uint8_t *data, uint16_t size);
int8_t EEP_write (uint32_t addr, uint8_t *data, uint16_t size);

#endif
