/*
 * SOES Simple Open EtherCAT Slave
 *
 * Copyright (C) 2007-2015 Arthur Ketels
 * Copyright (C) 2012-2015 rt-labs
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

#include <kern.h>
#include <bsp.h>

#include "utypes.h"
#include "esc.h"
#include "bootstrap.h"

extern uint32_t local_boot_state;

#define GPIO_LED_RED 1 /* Dummy GPIO number, suppose to be real GPIO pin for LED */

enum led_flash
{ OFF, ON, FLICKER, BLINK, SINGLE_FLASH, DOUBLE_FLASH, INVERTED_DOUBLE_FLASH,
      TRIPLE_FLASH };

void led_handler(uint32_t led_state, uint32_t led_number)
{
   switch (led_state)
   {
      case OFF:
      {
         gpio_set (led_number, 1);
         task_delay (tick_from_ms (1000));
         break;
      }
      case ON:
      {
         gpio_set (led_number, 0);
         task_delay (tick_from_ms (1000));
         break;
      }
      case FLICKER:
      {
         gpio_set (led_number, 0);
         task_delay (tick_from_ms (50));
         gpio_set (led_number, 1);
         task_delay (tick_from_ms (50));
         break;
      }
      case BLINK:
      {
         gpio_set (led_number, 0);
         task_delay (tick_from_ms (200));
         gpio_set (led_number, 1);
         task_delay (tick_from_ms (200));
         break;
      }
      case SINGLE_FLASH:
      {
         gpio_set (led_number, 0);
         task_delay (tick_from_ms (200));
         gpio_set (led_number, 1);
         task_delay (tick_from_ms (1000));
         break;
      }
      case DOUBLE_FLASH:
      {
         gpio_set (led_number, 0);
         task_delay (tick_from_ms (200));
         gpio_set (led_number, 1);
         task_delay (tick_from_ms (200));
         gpio_set (led_number, 0);
         task_delay (tick_from_ms (200));
         gpio_set (led_number, 1);
         task_delay (tick_from_ms (1000));
         break;
      }
      case INVERTED_DOUBLE_FLASH:
      case TRIPLE_FLASH:
      default:
      {
         task_delay (tick_from_ms (1000));
         break;
      }
   }
}
void led_run (void *arg)
{

   uint16_t al_status;
   enum led_flash led_state = OFF;

   for (;;)
   {
      /* Led states according to ETG1300 IndicatorLabelingSpecification
       * RUN led task only used if not ESC pin is connected.
       */
      al_status = ESCvar.ALstatus & 0x1f;
      if (al_status & ESCop)
      {
         led_state = ON;
      }
      else if (al_status & ESCinit)
      {
         led_state = OFF;
      }
      else if (al_status & ESCpreop)
      {
         led_state = BLINK;
      }
      else if (al_status & ESCsafeop)
      {
         led_state = SINGLE_FLASH;
      }
      else if (al_status & ESCboot)
      {
         led_state = FLICKER;
      }
      else
      {
         led_state = OFF;
      }

      led_handler (led_state, GPIO_LED_RED);
   }
}

void led_error (void *arg)
{

   uint16_t al_status,al_status_code;
   enum led_flash led_state = OFF;

   for (;;)
   {
      /* Led states according to ETG1300 IndicatorLabelingSpecification */
      al_status = ESCvar.ALstatus & 0x001f;
      al_status_code = ESCvar.ALerror;

      /* Note: Flicker is defined blow aswell
       * Capture APPLICATION errors that happend during boot
       */
      if ((al_status & ESCboot) && (local_boot_state == BOOT_FAILED))
      {
         led_state = FLICKER;
      }
      /* The AL status code == No error or NO AL status Error Indication
       * of not entered the requested starte
       */
      else if (al_status_code == 0x00 || !(al_status & 0x10))
      {
         led_state = OFF;
      }
      /* 0x002D No Sync Error
       * 0x0032 PLL error
       */
      else if (al_status_code == 0x2D || al_status_code == 0x32 )
      {
         led_state = SINGLE_FLASH;
      }
      /* 0x001B SyncManager watchdog */
      else if (al_status_code == 0x1B )
      {
         led_state = DOUBLE_FLASH;
      }
      /* Booting error, Init state reached but Error Indicator bit is set
       * to 1 in Al Status
       * Application BOOT information, fail or not
       */
      else if (al_status & (ESCinit | ESCerror))
      {
         led_state = FLICKER;
      }
      /* Invalid Configuration, General configuration change
       * We'll end here at last if we got errors in ALstatus
       */
      else
      {
         led_state = BLINK;
      }
      led_handler (led_state,GPIO_LED_RED);
   }
}
