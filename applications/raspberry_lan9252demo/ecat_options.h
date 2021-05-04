#ifndef __ECAT_OPTIONS_H__
#define __ECAT_OPTIONS_H__

#include "cc.h"

#define DYN_PDO_MAPPING  1

#define USE_MBX          0
#define USE_FOE          0
#define USE_EOE          0

#if USE_MBX
#define MBXSIZE          128
#define MBXSIZEBOOT      128
#else
#define MBXSIZE          0
#define MBXSIZEBOOT      0
#endif
#define MBXBUFFERS       3

#define SM0_sma          0x1000
#define SM0_smc          0x64
#define SM0_act          1
#define SM1_sma          0x1200
#define SM1_smc          0x20
#define SM1_act          1

#define MBX0_sma         SM0_sma
#define MBX0_sml         MBXSIZE
#define MBX0_sme         MBX0_sma+MBX0_sml-1
#define MBX0_smc         SM0_smc
#define MBX1_sma         SM1_sma
#define MBX1_sml         MBXSIZE
#define MBX1_sme         MBX1_sma+MBX1_sml-1
#define MBX1_smc         SM1_act

#define MBX0_sma_b       SM0_sma
#define MBX0_sml_b       MBXSIZEBOOT
#define MBX0_sme_b       MBX0_sma_b+MBX0_sml_b-1
#define MBX0_smc_b       SM0_smc
#define MBX1_sma_b       SM1_sma
#define MBX1_sml_b       MBXSIZEBOOT
#define MBX1_sme_b       MBX1_sma_b+MBX1_sml_b-1
#define MBX1_smc_b       SM1_act

#define SM2_sma          SM0_sma
#define SM2_smc          SM0_smc
#define SM2_act          SM0_act
#define SM3_sma          SM1_sma
#define SM3_smc          SM1_smc
#define SM3_act          SM1_act

#define MAX_RXPDO_SIZE   128
#define MAX_TXPDO_SIZE   128

#if DYN_PDO_MAPPING
#define MAX_MAPPINGS_SM2 1
#define MAX_MAPPINGS_SM3 1
#else
#define MAX_MAPPINGS_SM2 0
#define MAX_MAPPINGS_SM3 0
#endif

#endif /* __ECAT_OPTIONS_H__ */
