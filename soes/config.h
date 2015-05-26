
#ifndef __config_h__
#define __config_h__

#define MBXSIZE     0x80
#define MBXSIZEBOOT 0x80
#define MBXBUFFERS  3

#define MBX0_sma    0x1000
#define MBX0_sml    MBXSIZE
#define MBX0_sme    MBX0_sma+MBX0_sml-1
#define MBX0_smc    0x26
#define MBX1_sma    0x1080
#define MBX1_sml    MBXSIZE
#define MBX1_sme    MBX1_sma+MBX1_sml-1
#define MBX1_smc    0x22

#define MBX0_sma_b  0x1000
#define MBX0_sml_b  MBXSIZEBOOT
#define MBX0_sme_b  MBX0_sma_b+MBX0_sml_b-1
#define MBX0_smc_b  0x26
#define MBX1_sma_b  0x1080
#define MBX1_sml_b  MBXSIZEBOOT
#define MBX1_sme_b  MBX1_sma_b+MBX1_sml_b-1
#define MBX1_smc_b  0x22

#define SM2_sma     0x1100
#define SM2_smc     0x24
#define SM2_act     0x01
#define SM3_sma     0x1180
#define SM3_smc     0x20
#define SM3_act     0x01

#endif
