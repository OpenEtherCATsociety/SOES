#ifndef __ECAT_OPTIONS_H__
#define __ECAT_OPTIONS_H__

#include "cc.h"

#define DYN_PDO_MAPPING  0

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

#define SM0_sma          0x1000
#define SM0_smc          0x64
#define SM0_act          1
#define SM1_sma          0x1200
#define SM1_smc          0x20
#define SM1_act          1

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
