/******************************************************************************
 *                *          ***                    ***
 *              ***          ***                    ***
 * ***  ****  **********     ***        *****       ***  ****          *****
 * *********  **********     ***      *********     ************     *********
 * ****         ***          ***              ***   ***       ****   ***
 * ***          ***  ******  ***      ***********   ***        ****   *****
 * ***          ***  ******  ***    *************   ***        ****      *****
 * ***          ****         ****   ***       ***   ***       ****          ***
 * ***           *******      ***** **************  *************    *********
 * ***             *****        ***   *******   **  **  ******         *****
 *                           t h e  r e a l t i m e  t a r g e t  e x p e r t s
 *
 * http://www.rt-labs.com
 * Copyright (C) 2012-2013. rt-labs AB, Sweden. All rights reserved.
 *------------------------------------------------------------------------------
 * $Id: main.c 522 2013-06-20 16:16:45Z rtlaka $
 *------------------------------------------------------------------------------
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
