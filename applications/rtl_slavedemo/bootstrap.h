/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
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
