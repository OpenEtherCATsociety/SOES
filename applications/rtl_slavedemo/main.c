/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
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
