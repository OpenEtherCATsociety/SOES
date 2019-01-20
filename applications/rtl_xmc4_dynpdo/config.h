#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <cc.h>

#define MAX_INPUT_SIZE   128    /* Max processdata size (inputs) */
#define MAX_OUTPUT_SIZE  128    /* Max processdata size (outputs) */

#define MAX_PDO          2      /* Max number of writeable PDOs (RX + TX) */
#define MAX_PDO_ENTRIES  4      /* Max number of entries in a writeable PDO */

#define MAX_SM_ENTRIES   4      /* Max number of PDOs in a SyncManager */
#define MAX_SM_MAPPINGS  16     /* Max number of mapped objects in a SyncManager */

#define MBXSIZE     128
#define MBXSIZEBOOT 128
#define MBXBUFFERS  3

#define MBX0_sma    0x1000
#define MBX0_sml    MBXSIZE
#define MBX0_sme    MBX0_sma+MBX0_sml-1
#define MBX0_smc    0x26
#define MBX1_sma    MBX0_sma+MBX0_sml
#define MBX1_sml    MBXSIZE
#define MBX1_sme    MBX1_sma+MBX1_sml-1
#define MBX1_smc    0x22

#define MBX0_sma_b  0x1000
#define MBX0_sml_b  MBXSIZEBOOT
#define MBX0_sme_b  MBX0_sma_b+MBX0_sml_b-1
#define MBX0_smc_b  0x26
#define MBX1_sma_b  MBX0_sma_b+MBX0_sml_b
#define MBX1_sml_b  MBXSIZEBOOT
#define MBX1_sme_b  MBX1_sma_b+MBX1_sml_b-1
#define MBX1_smc_b  0x22

#define SM2_sma     0x1100
#define SM2_smc     0x24
#define SM2_act     1
#define SM3_sma     0x1180
#define SM3_smc     0x20
#define SM3_act     1

#endif /* __CONFIG_H__ */
