#ifndef __ECAT_OPTIONS_H__
#define __ECAT_OPTIONS_H__

#include "cc.h"

#define USE_FOE          1
#define USE_EOE          0

#define MBXSIZE          128
#define MBXSIZEBOOT      128
#define MBXBUFFERS       3

#define MBX0_sma         0x1000
#define MBX0_sml         MBXSIZE
#define MBX0_sme         MBX0_sma+MBX0_sml-1
#define MBX0_smc         0x26
#define MBX1_sma         MBX0_sma+MBX0_sml
#define MBX1_sml         MBXSIZE
#define MBX1_sme         MBX1_sma+MBX1_sml-1
#define MBX1_smc         0x22

#define MBX0_sma_b       0x1000
#define MBX0_sml_b       MBXSIZEBOOT
#define MBX0_sme_b       MBX0_sma_b+MBX0_sml_b-1
#define MBX0_smc_b       0x26
#define MBX1_sma_b       MBX0_sma_b+MBX0_sml_b
#define MBX1_sml_b       MBXSIZEBOOT
#define MBX1_sme_b       MBX1_sma_b+MBX1_sml_b-1
#define MBX1_smc_b       0x22

#define SM2_sma          0x1100
#define SM2_smc          0x24
#define SM2_act          1
#define SM3_sma          0x1180
#define SM3_smc          0x20
#define SM3_act          1

#define MAX_RXPDO_SIZE   42
#define MAX_TXPDO_SIZE   42

#define MAX_MAPPINGS_SM2 2
#define MAX_MAPPINGS_SM3 1

#endif /* __ECAT_OPTIONS_H__ */
