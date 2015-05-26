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

int main(void)
{
   extern void led_run (void *arg);
   extern void led_error (void *arg);
   extern void soes (void *arg);
   extern void my_cyclic_callback (void * arg);

   /* task_spawn ("led_run", led_run, 15, 512, NULL); */
   task_spawn ("led_error", led_error, 15, 512, NULL);
   task_spawn ("t_StatsPrint", my_cyclic_callback, 20, 1024, (void *)NULL);
   task_spawn ("soes", soes, 8, 1024, NULL);

   return (0);
}
