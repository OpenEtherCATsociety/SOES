/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <kern.h>
#include <bsp.h>
#include <flash_drv.h>

#include <esc_foe.h>
#include "bootstrap.h"

uint32_t local_boot_state        = BOOT_IDLE;
static uint32_t boot_started     = 0;
static uint32_t boot_watch_dog   = 0;
static uint32_t boot_time_ms     = BOOT_WATCHDOG_MS;
static uint32_t current_tick_ms;

/* Dummy values pointing to a flash writing area when updating Firmware */
#define FLASH_BLOCK_SIZE      256 * 1024
#define FLASH_SECTOR_SIZE     2048
#define FLASH_EEPROM_SECTIONS 2

void boot_soft_reset (void)
{
   DPRINT ("boot_soft_reset\n");
}

void init_boothook (void)
{
   DPRINT("osal_init_boothook\n");
   local_boot_state = BOOT_START;
}

void boot_inithook (void)
{
   if (local_boot_state == BOOT_FAILED)
   {
      DPRINT("osal_boot_inithook : BOOT failed\n");
      local_boot_state = BOOT_IDLE;
   }
   else
   {
      DPRINT("osal_boot_inithook\n");
      local_boot_state = BOOT_SWAP;
   }
}

uint32_t flash_foe_buffer (foe_file_cfg_t * self, uint8_t * data, size_t length)
{
   uint32_t flash_cmd_failed = 0;
   uint32_t calculated_address = self->dest_start_address + self->address_offset;

   /* This part is Cortex M4 Kinetis specific therefore placed in Hooks*/
   /* Erase every new sector we enter by looking at modulo sector size */
   if ( (calculated_address %  FLASH_SECTOR_SIZE) == 0)
   {
      //FIXME: flash_cmd_failed = flash_drv_erase_sector (calculated_address);
      DPRINT("flash_erase_sector %x\n",calculated_address);
      if(flash_cmd_failed)
      {
         DPRINT("flash_erase_sector %x\n",calculated_address);
      }
   }
   /* Program flash */
   if (!flash_cmd_failed)
   {
      //FIXME: flash_cmd_failed = flash_drv_program_longword (calculated_address, data);
      if(flash_cmd_failed)
      {
         DPRINT("flash_program_failed %x\n",calculated_address);
      }
   }

   /* Check the newly programmed flash */
   if (!flash_cmd_failed)
   {
      flash_cmd_failed = flash_drv_program_check (calculated_address,
                                                  FTFL_USER_MARGIN, data);
      if(flash_cmd_failed)
      {
         DPRINT("flash_check failed at %x\n",calculated_address);
      }
   }

   return flash_cmd_failed;
}

void bootstrap_foe_init  (void)
{
   /* Imported linker script variable
    * extern uint32_t flash_start;
    */
   /* This part is Cortex M4 Kinetis specific therefore placed in Hooks*/
   static foe_file_cfg_t files[] =
   {
      {
         .name               = "ato.bin",
         .max_data           = FLASH_BLOCK_SIZE,
         .dest_start_address = FLASH_BLOCK_SIZE + 0, /* + (uint32_t)&flash_start,*/
         .address_offset     = 0,
         .filepass           = 0,
         .write_only_in_boot = 1,
         .write_function     = flash_foe_buffer   /* NULL if not used */
      },
      {
         .name               = "eeprom.bin",
         .max_data           = FLASH_SECTOR_SIZE,
         .dest_start_address = (2 * FLASH_BLOCK_SIZE) -
                               (FLASH_EEPROM_SECTIONS * FLASH_SECTOR_SIZE),
         .address_offset     = 0,
         .filepass           = 0,
         .write_only_in_boot = 1,
         .write_function     = flash_foe_buffer  /* NULL if not used */
      },
   };

   static uint8_t fbuf[FLASH_WRITE_BLOCK_SIZE];
   static foe_cfg_t config =
   {
      .buffer_size = FLASH_WRITE_BLOCK_SIZE,  /* Buffer size before we flush to destination */
      .fbuffer     = (uint8_t *)&fbuf,
      .n_files     = NELEMENTS (files),
      .files       = files
   };

   FOE_config ((foe_cfg_t *)&config);
}

void bootstrap_state (void)
{
   switch (local_boot_state)
   {
      /* BOOT_START is set from Init->Boot hook called from ESC_state */
      case BOOT_START:
      {
         local_boot_state = BOOT_UPDATING;
         boot_started = tick_to_ms (tick_get ());
         boot_watch_dog = boot_started + boot_time_ms;
         DPRINT ("boot_started: %d, boot_watch_dog: %d\n",boot_started,boot_watch_dog);
         break;
      }
      case BOOT_UPDATING:
      {
         current_tick_ms = tick_to_ms (tick_get ());
         if (boot_watch_dog <  current_tick_ms)
         {
            local_boot_state = BOOT_FAILED;
            DPRINT ("boot failed at: %d\n",current_tick_ms);
         }
         break;
      }
      case BOOT_SWAP:
      {
         /* BOOT_SWAP is set from Boot->Init hook called from ESC_state */
         //FIXME:if (!boot_swap_flash ())
         /*FIXME:*/if (1)
         {
            /* IF it went OK we'll trigger BOOT_RESET */
            local_boot_state = BOOT_RESET;
         }
         else
         {
            DPRINT ("boot_swap_failed\n");
            local_boot_state = BOOT_FAILED;
         }
         break;
      }
      case BOOT_RESET:
      {
         /* BOOT_RESET is set from Boot->Init hook called from ESC_state */
         local_boot_state = BOOT_IDLE;
         boot_soft_reset();
         break;
      }
      case BOOT_FAILED:
      default:
         break;
   }
}
