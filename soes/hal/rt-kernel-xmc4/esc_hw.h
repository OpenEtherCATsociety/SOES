/*
 * SOES Simple Open EtherCAT Slave
 *
 * File    : esc_hw_eep.h
 * Version : 1.0.0
 * Date    : 26-08-2016
 * Copyright (C) 2016 Sascha Ittner
 *
 * SOES is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * SOES is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * As a special exception, if other files instantiate templates or use macros
 * or inline functions from this file, or you compile this file and link it
 * with other works to produce a work based on this file, this file does not
 * by itself cause the resulting work to be covered by the GNU General Public
 * License. However the source code for this file must still be made available
 * in accordance with section (3) of the GNU General Public License.
 *
 * This exception does not invalidate any other reasons why a work based on
 * this file might be covered by the GNU General Public License.
 *
 * The EtherCAT Technology, the trade name and logo "EtherCAT" are the intellectual
 * property of, and protected by Beckhoff Automation GmbH.
 */

 /** \file
 * \brief
 * ESC hardware specifoc EEPROM emulation functions.
 */

#ifndef __esc_hw__
#define __esc_hw__

#include <kern.h>
/* ================================================================================ */
/* ================                  ECAT [ECAT0]                  ================ */
/* ================================================================================ */


/**
  * @brief EtherCAT 0 (ECAT)
  */

typedef struct esc_registers
{                                    /*!< (@ 0x54010000) ECAT Structure                                         */
   uint8_t   TYPE;                              /*!< (@ 0x54010000) Type of EtherCAT Controller                            */
   uint8_t   REVISION;                          /*!< (@ 0x54010001) Revision of EtherCAT Controller                        */
   uint16_t  BUILD;                             /*!< (@ 0x54010002) Build Version                                          */
   uint8_t   FMMU_NUM;                          /*!< (@ 0x54010004) FMMUs Supported                                        */
   uint8_t   SYNC_MANAGER;                      /*!< (@ 0x54010005) SyncManagers Supported                                 */
   uint8_t   RAM_SIZE;                          /*!< (@ 0x54010006) RAM Size                                               */
   uint8_t   PORT_DESC;                         /*!< (@ 0x54010007) Port Descriptor                                        */
   uint16_t  FEATURE;                           /*!< (@ 0x54010008) ESC Features Supported                                 */
   uint16_t  RESERVED[3];
   uint16_t  STATION_ADR;                       /*!< (@ 0x54010010) Configured Station Address                             */
   uint16_t  STATION_ALIAS;                     /*!< (@ 0x54010012) Configured Station Alias                               */
   uint32_t  RESERVED1[3];
   uint8_t   WR_REG_ENABLE;                     /*!< (@ 0x54010020) Write Register Enable                                  */
   uint8_t   WR_REG_PROTECT;                    /*!< (@ 0x54010021) Write Register Protection                              */
   uint16_t  RESERVED2[7];
   uint8_t   ESC_WR_ENABLE;                     /*!< (@ 0x54010030) ESC Write Enable                                       */
   uint8_t   ESC_WR_PROTECT;                    /*!< (@ 0x54010031) ESC Write Protection                                   */
   uint16_t  RESERVED3[7];
   union
   {
      uint8_t   ESC_RESET_ECAT_READMode;         /*!< (@ 0x54010040) ESC Reset ECAT [READ Mode]                             */
      uint8_t   ESC_RESET_ECAT_WRITEMode;        /*!< (@ 0x54010040) ESC Reset ECAT [WRITE Mode]                            */
   };
   union {
   uint8_t   ESC_RESET_PDI_READMode;          /*!< (@ 0x54010041) ESC Reset PDI [READ Mode]                              */
   uint8_t   ESC_RESET_PDI_WRITEMode;         /*!< (@ 0x54010041) ESC Reset PDI [WRITE Mode]                             */
   };
   uint16_t  RESERVED4[95];
   uint32_t  ESC_DL_CONTROL;                    /*!< (@ 0x54010100) ESC DL Control                                         */
   uint32_t  RESERVED5;
   uint16_t  PHYSICAL_RW_OFFSET;                /*!< (@ 0x54010108) Physical Read/Write Offset                             */
   uint16_t  RESERVED6[3];
   uint16_t  ESC_DL_STATUS;                     /*!< (@ 0x54010110) ESC DL Status                                          */
   uint16_t  RESERVED7[7];
   uint16_t  AL_CONTROL;                        /*!< (@ 0x54010120) AL Control                                             */
   uint16_t  RESERVED8[7];
   uint16_t  AL_STATUS;                         /*!< (@ 0x54010130) AL Status                                              */
   uint16_t  RESERVED9;
   uint16_t  AL_STATUS_CODE;                    /*!< (@ 0x54010134) AL Status Code                                         */
   uint16_t  RESERVED10;
   uint8_t   RUN_LED;                           /*!< (@ 0x54010138) RUN LED Override                                       */
   uint8_t   ERR_LED;                           /*!< (@ 0x54010139) RUN ERR Override                                       */
   uint16_t  RESERVED11[3];
   uint8_t   PDI_CONTROL;                       /*!< (@ 0x54010140) PDI Control                                            */
   uint8_t   ESC_CONFIG;                        /*!< (@ 0x54010141) ESC Configuration                                      */
   uint16_t  RESERVED12[7];
   uint8_t   PDI_CONFIG;                        /*!< (@ 0x54010150) PDI Control                                            */
   uint8_t   SYNC_LATCH_CONFIG;                 /*!< (@ 0x54010151) Sync/Latch[1:0] PDI Configuration                      */
   uint16_t  PDI_EXT_CONFIG;                    /*!< (@ 0x54010152) PDI Synchronous Microcontroller extended Configuration */
   uint32_t  RESERVED13[43];
   uint16_t  EVENT_MASK;                        /*!< (@ 0x54010200) ECAT Event Mask                                        */
   uint16_t  RESERVED14;
   uint32_t  AL_EVENT_MASK;                     /*!< (@ 0x54010204) PDI AL Event Mask                                      */
   uint32_t  RESERVED15[2];
   uint16_t  EVENT_REQ;                         /*!< (@ 0x54010210) ECAT Event Request                                     */
   uint16_t  RESERVED16[7];
   uint32_t  AL_EVENT_REQ;                      /*!< (@ 0x54010220) AL Event Request                                       */
   uint32_t  RESERVED17[55];
   uint16_t  RX_ERR_COUNT0;                     /*!< (@ 0x54010300) RX Error Counter Port 0                                */
   uint16_t  RX_ERR_COUNT1;                     /*!< (@ 0x54010302) RX Error Counter Port 1                                */
   uint32_t  RESERVED18;
   uint8_t   FWD_RX_ERR_COUNT0;                 /*!< (@ 0x54010308) Forwarded RX Error Counter Port 0                      */
   uint8_t   FWD_RX_ERR_COUNT1;                 /*!< (@ 0x54010309) Forwarded RX Error Counter Port 1                      */
   uint16_t  RESERVED19;
   uint8_t   PROC_ERR_COUNT;                    /*!< (@ 0x5401030C) ECAT Processing Unit Error Counter                     */
   uint8_t   PDI_ERR_COUNT;                     /*!< (@ 0x5401030D) PDI Error Counter                                      */
   uint16_t  RESERVED20;
   uint8_t   LOST_LINK_COUNT0;                  /*!< (@ 0x54010310) Lost Link Counter Port 0                               */
   uint8_t   LOST_LINK_COUNT1;                  /*!< (@ 0x54010311) Lost Link Counter Port 1                               */
   uint16_t  RESERVED21[119];
   uint16_t  WD_DIVIDE;                         /*!< (@ 0x54010400) Watchdog Divider                                       */
   uint16_t  RESERVED22[7];
   uint16_t  WD_TIME_PDI;                       /*!< (@ 0x54010410) Watchdog Time PDI                                      */
   uint16_t  RESERVED23[7];
   uint16_t  WD_TIME_PDATA;                     /*!< (@ 0x54010420) Watchdog Time Process Data                             */
   uint16_t  RESERVED24[15];
   uint16_t  WD_STAT_PDATA;                     /*!< (@ 0x54010440) Watchdog Status Process Data                           */
   uint8_t   WD_COUNT_PDATA;                    /*!< (@ 0x54010442) Watchdog Counter Process Data                          */
   uint8_t   WD_COUNT_PDI;                      /*!< (@ 0x54010443) Watchdog Counter PDI                                   */
   uint32_t  RESERVED25[47];
   uint8_t   EEP_CONF;                          /*!< (@ 0x54010500) EEPROM Configuration                                   */
   uint8_t   EEP_STATE;                         /*!< (@ 0x54010501) EEPROM PDI Access State                                */
   uint16_t  EEP_CONT_STAT;                     /*!< (@ 0x54010502) EEPROM Control/Status                                  */
   uint32_t  EEP_ADR;                           /*!< (@ 0x54010504) EEPROM Address                                         */
   uint32_t  EEP_DATA[2];                       /*!< (@ 0x54010508) EEPROM Read/Write data                                 */
   uint16_t  MII_CONT_STAT;                     /*!< (@ 0x54010510) MII Management Control/Status                          */
   uint8_t   MII_PHY_ADR;                       /*!< (@ 0x54010512) PHY Address                                            */
   uint8_t   MII_PHY_REG_ADR;                   /*!< (@ 0x54010513) PHY Register Address                                   */
   uint16_t  MII_PHY_DATA;                      /*!< (@ 0x54010514) PHY Data                                               */
   uint8_t   MII_ECAT_ACS_STATE;                /*!< (@ 0x54010516) MII ECAT ACS STATE                                     */
   uint8_t   MII_PDI_ACS_STATE;                 /*!< (@ 0x54010517) MII PDI ACS STATE                                      */
   uint32_t  RESERVED26[250];
   uint32_t  DC_RCV_TIME_PORT0;                 /*!< (@ 0x54010900) Receive Time Port 0                                    */
   uint32_t  DC_RCV_TIME_PORT1;                 /*!< (@ 0x54010904) Receive Time Port 1                                    */
   uint32_t  RESERVED27[2];
   union
   {
      uint32_t  READMode_DC_SYS_TIME[2];         /*!< (@ 0x54010910) System Time read access                                */
      uint32_t  DC_SYS_TIME_WRITEMode;           /*!< (@ 0x54010910) System Time [WRITE Mode]                               */
   };
   uint32_t  RECEIVE_TIME_PU[2];                /*!< (@ 0x54010918) Local time of the beginning of a frame                 */
   uint32_t  DC_SYS_TIME_OFFSET[2];             /*!< (@ 0x54010920) Difference between local time and System Time          */
   uint32_t  DC_SYS_TIME_DELAY;                 /*!< (@ 0x54010928) System Time Delay                                      */
   uint32_t  DC_SYS_TIME_DIFF;                  /*!< (@ 0x5401092C) System Time Difference                                 */
   uint16_t  DC_SPEED_COUNT_START;              /*!< (@ 0x54010930) Speed Counter Start                                    */
   uint16_t  DC_SPEED_COUNT_DIFF;               /*!< (@ 0x54010932) Speed Counter Diff                                     */
   uint8_t   DC_SYS_TIME_FIL_DEPTH;             /*!< (@ 0x54010934) System Time Difference Filter Depth                    */
   uint8_t   DC_SPEED_COUNT_FIL_DEPTH;          /*!< (@ 0x54010935) Speed Counter Filter Depth                             */
   uint16_t  RESERVED28[37];
   uint8_t   DC_CYC_CONT;                       /*!< (@ 0x54010980) Cyclic Unit Control                                    */
   uint8_t   DC_ACT;                            /*!< (@ 0x54010981) Activation register                                    */
   uint16_t  DC_PULSE_LEN;                      /*!< (@ 0x54010982) Pulse Length of SyncSignals                            */
   uint8_t   DC_ACT_STAT;                       /*!< (@ 0x54010984) Activation Status                                      */
   uint8_t   RESERVED29[9];
   uint8_t   DC_SYNC0_STAT;                     /*!< (@ 0x5401098E) SYNC0 Status                                           */
   uint8_t   DC_SYNC1_STAT;                     /*!< (@ 0x5401098F) SYNC1 Status                                           */
   uint32_t  DC_CYC_START_TIME[2];              /*!< (@ 0x54010990) Start Time Cyclic Operation                            */
   uint32_t  DC_NEXT_SYNC1_PULSE[2];            /*!< (@ 0x54010998) System time of next SYNC1 pulse in ns                  */
   uint32_t  DC_SYNC0_CYC_TIME;                 /*!< (@ 0x540109A0) SYNC0 Cycle Time                                       */
   uint32_t  DC_SYNC1_CYC_TIME;                 /*!< (@ 0x540109A4) SYNC1 Cycle Time                                       */
   uint8_t   DC_LATCH0_CONT;                    /*!< (@ 0x540109A8) Latch0 Control                                         */
   uint8_t   DC_LATCH1_CONT;                    /*!< (@ 0x540109A9) Latch1 Control                                         */
   uint32_t  RESERVED30;
   uint8_t   DC_LATCH0_STAT;                    /*!< (@ 0x540109AE) Latch0 Status                                          */
   uint8_t   DC_LATCH1_STAT;                    /*!< (@ 0x540109AF) Latch1 Status                                          */
   uint32_t  DC_LATCH0_TIME_POS[2];             /*!< (@ 0x540109B0) Register captures System time at the positive
                                                         edge of the Latch0 signal                                             */
   uint32_t  DC_LATCH0_TIME_NEG[2];             /*!< (@ 0x540109B8) Register captures System time at the negative
                                                         edge of the Latch0 signal                                             */
   uint32_t  DC_LATCH1_TIME_POS[2];             /*!< (@ 0x540109C0) Register captures System time at the positive
                                                         edge of the Latch1 signal                                             */
   uint32_t  DC_LATCH1_TIME_NEG[2];             /*!< (@ 0x540109C8) Register captures System time at the negative
                                                         edge of the Latch1 signal                                             */
   uint32_t  RESERVED31[8];
   uint32_t  DC_ECAT_CNG_EV_TIME;               /*!< (@ 0x540109F0) EtherCAT Buffer Change Event Time                      */
   uint32_t  RESERVED32;
   uint32_t  DC_PDI_START_EV_TIME;              /*!< (@ 0x540109F8) PDI Buffer Start Event Time                            */
   uint32_t  DC_PDI_CNG_EV_TIME;                /*!< (@ 0x540109FC) PDI Buffer Change Event Time                           */
   uint32_t  RESERVED33[256];
   uint32_t  ID;                                /*!< (@ 0x54010E00) ECAT0 Module ID                                        */
   uint32_t  RESERVED34;
   uint32_t  STATUS;                            /*!< (@ 0x54010E08) ECAT0 Status                                           */
} esc_registers_t;


/* periodic task */
void EEP_hw_process (void);

#if SOES_IRQ
#define ATOMIC_SET(var,val)   __atomic_store_n(&var,val,__ATOMIC_SEQ_CST)
#define ATOMIC_GET(var)       __atomic_load_n(&var,__ATOMIC_SEQ_CST)
#define ATOMIC_ADD(var,val)   __atomic_add_fetch(&var,val,__ATOMIC_SEQ_CST)
#define ATOMIC_SUB(var,val)   __atomic_sub_fetch(&var,val,__ATOMIC_SEQ_CST)
#define ATOMIC_AND(var,val)   __atomic_and_fetch(&var,val,__ATOMIC_SEQ_CST)
#define ATOMIC_OR(var,val)    __atomic_or_fetch(&var,val,__ATOMIC_SEQ_CST)
#endif

extern sem_t * ecat_isr_sem;
void ecat_isr(void * arg);

#endif

