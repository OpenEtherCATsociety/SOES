/*
 * SOES Simple Open EtherCAT Slave
 *
 * File    : esc.h
 * Version : 0.9.2
 * Date    : 22-02-2010
 * Copyright (C) 2007-2013 Arthur Ketels
 * Copyright (C) 2012-2013 rt-labs.
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
 * Headerfile for esc.h
 */

#ifndef __esc__
#define __esc__

/* storage modifier for data stored in flashmemory */
#define FLASHSTORE               const
#define PACKED                   __attribute__((__packed__))

#define ESCREG_ADDRESS           0x0010
#define ESCREG_DLSTATUS          0x0110
#define ESCREG_ALCONTROL         0x0120
#define ESCREG_ALSTATUS          0x0130
#define ESCREG_ALERROR           0x0134
#define ESCREG_ALEVENT_SM_MASK   0x0310
#define ESCREG_ALEVENT_SMCHANGE  0x0010
#define ESCREG_ALEVENT_CONTROL   0x0001
#define ESCREG_ALEVENT_SM2       0x0400
#define ESCREG_ALEVENT_SM3       0x0800
#define ESCREG_WDSTATUS          0x0440
#define ESCREG_SM0               0x0800
#define ESCREG_SM0STATUS         (ESCREG_SM0 + 5)
#define ESCREG_SM0PDI            (ESCREG_SM0 + 7)
#define ESCREG_SM1               (ESCREG_SM0 + 0x08)
#define ESCREG_SM2               (ESCREG_SM0 + 0x10)
#define ESCREG_SM3               (ESCREG_SM0 + 0x18)
#define ESCREG_LOCALTIME         0x0910
#define ESCREG_SMENABLE_BIT      0x01
#define ESCREG_AL_STATEMASK      0x001f
#define ESCREG_AL_ALLBUTINITMASK 0x0e
#define ESCREG_AL_ERRACKMASK     0x0f

#define ESCinit                  0x01
#define ESCpreop                 0x02
#define ESCboot                  0x03
#define ESCsafeop                0x04
#define ESCop                    0x08
#define ESCerror                 0x10

#define INIT_TO_INIT             0x11
#define INIT_TO_PREOP            0x21
#define INIT_TO_BOOT             0x31
#define INIT_TO_SAFEOP           0x41
#define INIT_TO_OP               0x81
#define PREOP_TO_INIT            0x12
#define PREOP_TO_PREOP           0x22
#define PREOP_TO_BOOT            0x32
#define PREOP_TO_SAFEOP          0x42
#define PREOP_TO_OP              0x82
#define BOOT_TO_INIT             0x13
#define BOOT_TO_PREOP            0x23
#define BOOT_TO_BOOT             0x33
#define BOOT_TO_SAFEOP           0x43
#define BOOT_TO_OP               0x83
#define SAFEOP_TO_INIT           0x14
#define SAFEOP_TO_PREOP          0x24
#define SAFEOP_TO_BOOT           0x34
#define SAFEOP_TO_SAFEOP         0x44
#define SAFEOP_TO_OP             0x84
#define OP_TO_INIT               0x18
#define OP_TO_PREOP              0x28
#define OP_TO_BOOT               0x38
#define OP_TO_SAFEOP             0x48
#define OP_TO_OP                 0x88

#define ALERR_NONE                  0x0000
#define ALERR_INVALIDSTATECHANGE    0x0011
#define ALERR_UNKNOWNSTATE          0x0012
#define ALERR_BOOTNOTSUPPORTED      0x0013
#define ALERR_INVALIDBOOTMBXCONFIG  0x0015
#define ALERR_INVALIDMBXCONFIG      0x0016
#define ALERR_INVALIDSMCONFIG       0x0017
#define ALERR_WATCHDOG              0x001B
#define ALERR_INVALIDOUTPUTSM       0x001D
#define ALERR_INVALIDINPUTSM        0x001E

#define MBXERR_SYNTAX                   0x0001
#define MBXERR_UNSUPPORTEDPROTOCOL      0x0002
#define MBXERR_INVALIDCHANNEL           0x0003
#define MBXERR_SERVICENOTSUPPORTED      0x0004
#define MBXERR_INVALIDHEADER            0x0005
#define MBXERR_SIZETOOSHORT             0x0006
#define MBXERR_NOMOREMEMORY             0x0007
#define MBXERR_INVALIDSIZE              0x0008

#define ABORT_NOTOGGLE                  0x05030000
#define ABORT_UNKNOWN                   0x05040001
#define ABORT_UNSUPPORTED               0x06010000
#define ABORT_WRITEONLY                 0x06010001
#define ABORT_READONLY                  0x06010002
#define ABORT_NOOBJECT                  0x06020000
#define ABORT_TYPEMISMATCH              0x06070010
#define ABORT_NOSUBINDEX                0x06090011
#define ABORT_GENERALERROR              0x08000000
#define ABORT_NOTINTHISSTATE            0x08000022

#define MBXstate_idle                   0x00
#define MBXstate_inclaim                0x01
#define MBXstate_outclaim               0x02
#define MBXstate_outreq                 0x03
#define MBXstate_outpost                0x04
#define MBXstate_backup                 0x05
#define MBXstate_again                  0x06

#define COE_DEFAULTLENGTH               0x0a
#define COE_HEADERSIZE                  0x0a
#define COE_SEGMENTHEADERSIZE           0x03
#define COE_SDOREQUEST                  0x02
#define COE_SDORESPONSE                 0x03
#define COE_SDOINFORMATION              0x08
#define COE_COMMAND_SDOABORT            0x80
#define COE_COMMAND_UPLOADREQUEST       0x40
#define COE_COMMAND_UPLOADRESPONSE      0x40
#define COE_COMMAND_UPLOADSEGMENT       0x00
#define COE_COMMAND_UPLOADSEGREQ        0x60
#define COE_COMMAND_DOWNLOADRESPONSE    0x60
#define COE_COMMAND_LASTSEGMENTBIT      0x01
#define COE_SIZE_INDICATOR              0x01
#define COE_EXPEDITED_INDICATOR         0x02
#define COE_COMPLETEACCESS              0x10
#define COE_TOGGLEBIT                   0x10
#define COE_INFOERROR                   0x07
#define COE_GETODLISTRESPONSE           0x02
#define COE_GETODRESPONSE               0x04
#define COE_ENTRYDESCRIPTIONRESPONSE    0x06
#define COE_VALUEINFO_ACCESS            0x01
#define COE_VALUEINFO_OBJECT            0x02
#define COE_VALUEINFO_MAPPABLE          0x04
#define COE_VALUEINFO_TYPE              0x08
#define COE_VALUEINFO_DEFAULT           0x10
#define COE_VALUEINFO_MINIMUM           0x20
#define COE_VALUEINFO_MAXIMUM           0x40

#define MBXSIZE                        0x80
#define MBXSIZEBOOT                    0x80
#define MBXBUFFERS                     3

#define MBX0_sma                       0x1000
#define MBX0_sml                       MBXSIZE
#define MBX0_sme                       MBX0_sma+MBX0_sml-1
#define MBX0_smc                       0x26
#define MBX1_sma                       0x1080
#define MBX1_sml                       MBXSIZE
#define MBX1_sme                       MBX1_sma+MBX1_sml-1
#define MBX1_smc                       0x22

#define MBX0_sma_b                     0x1000
#define MBX0_sml_b                     MBXSIZEBOOT
#define MBX0_sme_b                     MBX0_sma_b+MBX0_sml_b-1
#define MBX0_smc_b                     0x26
#define MBX1_sma_b                     0x1080
#define MBX1_sml_b                     MBXSIZEBOOT
#define MBX1_sme_b                     MBX1_sma_b+MBX1_sml_b-1
#define MBX1_smc_b                     0x22

#define SM2_sma                        0x1100
#define SM2_smc                        0x24
#define SM2_act                        0x01
#define SM3_sma                        0x1180
#define SM3_smc                        0x20
#define SM3_act                        0x01

#define MBXHSIZE                       sizeof(_MBXh)
#define MBXDSIZE                       MBXSIZE-MBXHSIZE

#define MBXERR                         0x00
#define MBXAOE                         0x01
#define MBXEOE                         0x02
#define MBXCOE                         0x03
#define MBXFOE                         0x04
#define MBXODL                         0x10
#define MBXOD                          0x20
#define MBXED                          0x30
#define MBXSEU                         0x40
#define MBXSED                         0x50

#define SMRESULT_ERRSM0                0x01
#define SMRESULT_ERRSM1                0x02
#define SMRESULT_ERRSM2                0x04
#define SMRESULT_ERRSM3                0x08

#define FOE_ERR_NOTDEFINED             0x8000
#define FOE_ERR_NOTFOUND               0x8001
#define FOE_ERR_ACCESS                 0x8002
#define FOE_ERR_DISKFULL               0x8003
#define FOE_ERR_ILLEGAL                0x8004
#define FOE_ERR_PACKETNO               0x8005
#define FOE_ERR_EXISTS                 0x8006
#define FOE_ERR_NOUSER                 0x8007
#define FOE_ERR_BOOTSTRAPONLY          0x8008
#define FOE_ERR_NOTINBOOTSTRAP         0x8009
#define FOE_ERR_NORIGHTS               0x800A
#define FOE_ERR_PROGERROR              0x800B

#define FOE_OP_RRQ                     1
#define FOE_OP_WRQ                     2
#define FOE_OP_DATA                    3
#define FOE_OP_ACK                     4
#define FOE_OP_ERR                     5
#define FOE_OP_BUSY                    6

#define FOE_READY                      0
#define FOE_WAIT_FOR_ACK               1
#define FOE_WAIT_FOR_FINAL_ACK         2
#define FOE_WAIT_FOR_DATA              3

#define EC_LITTLE_ENDIAN

// Attention! this struct is always little-endian
typedef struct PACKED
{
   uint16 PSA;
   uint16 Length;

#if defined(EC_LITTLE_ENDIAN)
   uint8 Mode:2;
   uint8 Direction:2;
   uint8 IntECAT:1;
   uint8 IntPDI:1;
   uint8 WTE:1;
   uint8 R1:1;

   uint8 IntW:1;
   uint8 IntR:1;
   uint8 R2:1;
   uint8 MBXstat:1;
   uint8 BUFstat:2;
   uint8 R3:2;

   uint8 ECsm:1;
   uint8 ECrep:1;
   uint8 ECr4:4;
   uint8 EClatchEC:1;
   uint8 EClatchPDI:1;

   uint8 PDIsm:1;
   uint8 PDIrep:1;
   uint8 PDIr5:6;
#endif

#if defined(EC_BIG_ENDIAN)
   uint8 R1:1;
   uint8 WTE:1;
   uint8 IntPDI:1;
   uint8 IntECAT:1;
   uint8 Direction:2;
   uint8 Mode:2;

   uint8 R3:2;
   uint8 BUFstat:2;
   uint8 MBXstat:1;
   uint8 R2:1;
   uint8 IntR:1;
   uint8 IntW:1;

   uint8 EClatchPDI:1;
   uint8 EClatchEC:1;
   uint8 ECr4:4;
   uint8 ECrep:1;
   uint8 ECsm:1;

   uint8 PDIr5:6;
   uint8 PDIrep:1;
   uint8 PDIsm:1;
#endif
} _ESCsm;

/* Attention! this struct is always little-endian */
typedef struct PACKED
{
   uint16 PSA;
   uint16 Length;
   uint8 Command;
   uint8 Status;
   uint8 ActESC;
   uint8 ActPDI;
} _ESCsm2;

typedef FLASHSTORE struct PACKED
{
   uint16 PSA;
   uint16 Length;
   uint8 Command;
} _ESCsmCompact;

typedef struct PACKED
{
   uint16 ALevent;
   uint16 ALstatus;
   uint16 ALcontrol;
   uint16 ALerror;
   uint16 DLstatus;
   uint16 address;
   uint8 mbxcnt;
   uint8 mbxincnt;
   uint8 mbxoutpost;
   uint8 mbxbackup;
   uint8 xoe;
   uint8 txcue;
   uint8 mbxfree;
   uint8 segmented;
   void *data;
   uint16 entries;
   uint16 frags;
   uint16 fragsleft;

#if defined(EC_LITTLE_ENDIAN)
   uint8 r1:1;
   uint8 toggle:1;
   uint8 r2:6;
#endif

#if defined(EC_BIG_ENDIAN)
   uint8 r2:6;
   uint8 toggle:1;
   uint8 r1:1;
#endif

   uint8 SMtestresult;
   int16 temp;
   uint16 wdcnt;
   uint32 PrevTime;
   uint32 Time;
   _ESCsm SM[4];
} _ESCvar;

typedef struct PACKED
{
   uint16 length;
   uint16 address;

#if defined(EC_LITTLE_ENDIAN)
   uint8 channel:6;
   uint8 priority:2;

   uint8 mbxtype:4;
   uint8 mbxcnt:4;
#endif

#if defined(EC_BIG_ENDIAN)
   uint8 priority:2;
   uint8 channel:6;

   uint8 mbxcnt:4;
   uint8 mbxtype:4;
#endif
} _MBXh;

typedef struct PACKED
{
   _MBXh header;
   uint8 b[MBXDSIZE];
} _MBX;

typedef struct PACKED
{
   uint16 numberservice;
} _COEh;

typedef struct PACKED
{
#if defined(EC_LITTLE_ENDIAN)
   uint8 opcode:7;
   uint8 incomplete:1;
#endif

#if defined(EC_BIG_ENDIAN)
   uint8 incomplete:1;
   uint8 opcode:7;
#endif

   uint8 reserved;
   uint16 fragmentsleft;
} _INFOh;

typedef struct PACKED
{
   _MBXh mbxheader;
   uint16 type;
   uint16 detail;
} _MBXerr;

typedef struct PACKED
{
   _MBXh mbxheader;
   _COEh coeheader;
   uint8 command;
   uint16 index;
   uint8 subindex;
   uint32 size;
} _COEsdo;

typedef struct PACKED
{
   _MBXh mbxheader;
   _COEh coeheader;
   _INFOh infoheader;
   uint16 index;
   uint16 datatype;
   uint8 maxsub;
   uint8 objectcode;
   char name;
} _COEobjdesc;

typedef struct PACKED
{
   _MBXh mbxheader;
   _COEh coeheader;
   _INFOh infoheader;
   uint16 index;
   uint8 subindex;
   uint8 valueinfo;
   uint16 datatype;
   uint16 bitlength;
   uint16 access;
   char name;
} _COEentdesc;

typedef struct PACKED
{
   uint8 opcode;
   uint8 reserved;
   union
   {
      uint32 password;
      uint32 packetnumber;
      uint32 errorcode;
   };
} _FOEh;

#define FOEHSIZE        (sizeof(_FOEh))
#define FOE_DATA_SIZE   (MBXSIZEBOOT - (MBXHSIZE+FOEHSIZE))

typedef struct PACKED
{
   _MBXh mbxheader;
   _FOEh foeheader;
   union
   {
      char filename[FOE_DATA_SIZE];
      uint8 data[FOE_DATA_SIZE];
      char errortext[FOE_DATA_SIZE];
   };
} _FOE;

/* state definition in mailbox
 * 0 : idle
 * 1 : claimed for inbox
 * 2 : claimed for outbox
 * 3 : request post outbox
 * 4 : outbox posted not send
 * 5 : backup outbox
 * 6 : mailbox needs to be transmitted again
 */
typedef struct
{
   uint8 state;
} _MBXcontrol;

typedef struct esc_cfg
{
   void (*pre_state_change_hook) (uint8 * as, uint8 * an);
   void (*post_state_change_hook) (uint8 * as, uint8 * an);
} esc_cfg_t;

void ESC_config (esc_cfg_t * cfg);
void ESC_ALerror (uint16 errornumber);
void ESC_ALstatus (uint8 status);
void ESC_SMstatus (uint8 n);
uint8 ESC_WDstatus (void);
uint8 ESC_claimbuffer (void);
uint8 ESC_startmbx (uint8 state);
void ESC_stopmbx (void);
void MBX_error (uint16 error);
uint8 ESC_mbxprocess (void);
void ESC_xoeprocess (void);
uint8 ESC_startinput (uint8 state);
void ESC_stopinput (void);
uint8 ESC_startoutput (uint8 state);
void ESC_stopoutput (void);
void ESC_ALevent (void);
void ESC_state (void);

/* From hardware file */
uint8 ESC_read (uint16 address, void *buf, uint16 len, void *tALevent);
uint8 ESC_write (uint16 address, void *buf, uint16 len, void *tALevent);
void ESC_init (void * arg);
/* From application */
extern void APP_safeoutput ();

extern volatile _ESCvar ESCvar;
extern _MBX MBX[MBXBUFFERS];
extern _MBXcontrol MBXcontrol[MBXBUFFERS];
extern uint8 MBXrun;
extern uint16 SM2_sml, SM3_sml;

typedef struct
{
   uint8 state;
} _App;

#define APPSTATE_IDLE      0x00
#define APPSTATE_INPUT     0x01
#define APPSTATE_OUTPUT    0x02

extern _App App;

#endif
