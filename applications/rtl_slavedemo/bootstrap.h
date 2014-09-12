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
 * $Id: bootstrap.h 522 2013-06-20 16:16:45Z rtlaka $
 *------------------------------------------------------------------------------
 */

#ifndef _bootstrap_
#define _bootstrap_

#define BOOT_IDLE                0
#define BOOT_START               1
#define BOOT_UPDATING            2
#define BOOT_SWAP                3
#define BOOT_RESET               4
#define BOOT_FAILED              99

#define BOOT_WATCHDOG_MS         20 * 1000

void bootstrap_foe_init (void);
void bootstrap_state (void);
void boot_inithook (void);
void init_boothook (void);

#endif
