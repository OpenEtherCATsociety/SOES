#ifndef __ECAT_OPTIONS_H__
#define __ECAT_OPTIONS_H__

#include "cc.h"

#define DYN_PDO_MAPPING  1

#define USE_FOE          0
#define USE_EOE          0

#define MBXSIZE          0
#define MBXSIZEBOOT      0
#define MBXBUFFERS       3

#define MBX0_sma         0x1000
#define MBX0_sml         MBXSIZE
#define MBX0_sme         MBX0_sma+MBX0_sml-1
#define MBX0_smc         0x64
#define MBX1_sma         0x1200
#define MBX1_sml         MBXSIZE
#define MBX1_sme         MBX1_sma+MBX1_sml-1
#define MBX1_smc         0x20

#define MBX0_sma_b       0x1000
#define MBX0_sml_b       MBXSIZEBOOT
#define MBX0_sme_b       MBX0_sma_b+MBX0_sml_b-1
#define MBX0_smc_b       0x64
#define MBX1_sma_b       0x1200
#define MBX1_sml_b       MBXSIZEBOOT
#define MBX1_sme_b       MBX1_sma_b+MBX1_sml_b-1
#define MBX1_smc_b       0x20

#define SM2_sma          0x1000
#define SM2_smc          0x64
#define SM2_act          1
#define SM3_sma          0x1200
#define SM3_smc          0x20
#define SM3_act          1

#define MAX_RXPDO_SIZE   128
#define MAX_TXPDO_SIZE   128

#ifdef DYN_PDO_BUFFER
#define MAX_MAPPINGS_SM2 1
#define MAX_MAPPINGS_SM3 1
#else
#define MAX_MAPPINGS_SM2 0
#define MAX_MAPPINGS_SM3 0
#endif

#endif /* __ECAT_OPTIONS_H__ */
