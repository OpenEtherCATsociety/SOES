/*
 * Minimal host ecat_options.h for the standalone SOES tests.
 * Licensed under the GNU General Public License version 2 with exceptions.
 */

#ifndef TEST_ECAT_OPTIONS_H
#define TEST_ECAT_OPTIONS_H

#include "cc.h"

#define USE_FOE          1
#define USE_EOE          0

#define MBXSIZE          128
#define MBXSIZEBOOT      128
#define MBXBUFFERS       3

#define MBX0_sma         0x1000
#define MBX0_sml         MBXSIZE
#define MBX0_sme         MBX0_sma + MBX0_sml - 1
#define MBX0_smc         0x26
#define MBX1_sma         MBX0_sma + MBX0_sml
#define MBX1_sml         MBXSIZE
#define MBX1_sme         MBX1_sma + MBX1_sml - 1
#define MBX1_smc         0x22

#define SM2_sma          0x1100
#define SM2_smc          0x24
#define SM2_act          1
#define SM3_sma          0x1180
#define SM3_smc          0x20
#define SM3_act          1

#endif
