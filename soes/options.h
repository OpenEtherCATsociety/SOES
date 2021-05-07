/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#ifndef __options__
#define __options__

/* User-defined options, Options defined here will override default values */
#include "ecat_options.h"

/* Device emulation support */
#ifndef USE_EMU
#define USE_EMU          0
#endif

/* Mailbox support */
/* If disabled SM2 --> SM0, and SM3 --> SM1 */
#ifndef USE_MBX
#define USE_MBX          1
#endif

/* FoE support */
#ifndef USE_FOE
#define USE_FOE          1
#endif

/* EoE support */
#ifndef USE_EOE
#define USE_EOE          1
#endif

#ifndef MBXSIZE
#define MBXSIZE          128
#endif

#ifndef MBXSIZEBOOT
#define MBXSIZEBOOT      128
#endif

#ifndef MBXBUFFERS
#define MBXBUFFERS       3
#endif

#ifndef PREALLOC_FACTOR
#define PREALLOC_FACTOR  3
#endif

#ifndef MBX0_sma
#define MBX0_sma         0x1000
#endif

#ifndef MBX0_sml
#define MBX0_sml         MBXSIZE
#endif

#ifndef MBX0_sme
#define MBX0_sme         MBX0_sma+MBX0_sml-1
#endif

#ifndef MBX0_smc
#define MBX0_smc         0x26
#endif

#ifndef MBX1_sma
#define MBX1_sma         MBX0_sma+MBX0_sml
#endif

#ifndef MBX1_sml
#define MBX1_sml         MBXSIZE
#endif

#ifndef MBX1_sme
#define MBX1_sme         MBX1_sma+MBX1_sml-1
#endif

#ifndef MBX1_smc
#define MBX1_smc         0x22
#endif

#ifndef MBX0_sma_b
#define MBX0_sma_b       0x1000
#endif

#ifndef MBX0_sml_b
#define MBX0_sml_b       MBXSIZEBOOT
#endif

#ifndef MBX0_sme_b
#define MBX0_sme_b       MBX0_sma_b+MBX0_sml_b-1
#endif

#ifndef MBX0_smc_b
#define MBX0_smc_b       0x26
#endif

#ifndef MBX1_sma_b
#define MBX1_sma_b       MBX0_sma_b+MBX0_sml_b
#endif

#ifndef MBX1_sml_b
#define MBX1_sml_b       MBXSIZEBOOT
#endif

#ifndef MBX1_sme_b
#define MBX1_sme_b       MBX1_sma_b+MBX1_sml_b-1
#endif

#ifndef MBX1_smc_b
#define MBX1_smc_b       0x22
#endif

#ifndef SM2_sma
#define SM2_sma          0x1100
#endif

#ifndef SM2_smc
#define SM2_smc          0x24
#endif

#ifndef SM2_act
#define SM2_act          1
#endif

#ifndef SM3_sma
#define SM3_sma          0x1180
#endif

#ifndef SM3_smc
#define SM3_smc          0x20
#endif

#ifndef SM3_act
#define SM3_act          1
#endif

/* Max number of dynamically mapped objects in SM2. May be 0 to
   disable dynamic processdata. */
#ifndef MAX_MAPPINGS_SM2
#define MAX_MAPPINGS_SM2 16
#endif

/* Max number of dynamically mapped objects in SM3. May be 0 to
   disable dynamic processdata. */
#ifndef MAX_MAPPINGS_SM3
#define MAX_MAPPINGS_SM3 16
#endif

/* Max processdata size (outputs). Only used if MAX_MAPPINGS_SM2 is
   non-zero. */
#ifndef MAX_RXPDO_SIZE
#define MAX_RXPDO_SIZE   128
#endif

/* Max processdata size (inputs). Only used if MAX_MAPPINGS_SM3 is
   non-zero. */
#ifndef MAX_TXPDO_SIZE
#define MAX_TXPDO_SIZE   128
#endif


#endif /* __options__ */
