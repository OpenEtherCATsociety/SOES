/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Headerfile for esc.h
 */

#ifndef __esc__
#define __esc__

#include <cc.h>

#define ESCREG_ADDRESS              0x0010
#define ESCREG_DLSTATUS             0x0110
#define ESCREG_ALCONTROL            0x0120
#define ESCREG_ALSTATUS             0x0130
#define ESCREG_ALERROR              0x0134
#define ESCREG_ALEVENTMASK          0x0204
#define ESCREG_ALEVENT              0x0220
#define ESCREG_ALEVENT_SM_MASK      0x0310
#define ESCREG_ALEVENT_SMCHANGE     0x0010
#define ESCREG_ALEVENT_CONTROL      0x0001
#define ESCREG_ALEVENT_DC_LATCH     0x0002
#define ESCREG_ALEVENT_DC_SYNC0     0x0004
#define ESCREG_ALEVENT_DC_SYNC1     0x0008
#define ESCREG_ALEVENT_EEP          0x0020
#define ESCREG_ALEVENT_SM0          0x0100
#define ESCREG_ALEVENT_SM1          0x0200
#define ESCREG_ALEVENT_SM2          0x0400
#define ESCREG_ALEVENT_SM3          0x0800
#define ESCREG_WDSTATUS             0x0440
#define ESCREG_EECONTSTAT           0x0502
#define ESCREG_EEDATA               0x0508
#define ESCREG_SM0                  0x0800
#define ESCREG_SM0STATUS            (ESCREG_SM0 + 5)
#define ESCREG_SM0PDI               (ESCREG_SM0 + 7)
#define ESCREG_SM1                  (ESCREG_SM0 + 0x08)
#define ESCREG_SM2                  (ESCREG_SM0 + 0x10)
#define ESCREG_SM3                  (ESCREG_SM0 + 0x18)
#define ESCREG_LOCALTIME            0x0910
#define ESCREG_SYNC_ACT             0x0981
#define ESCREG_SYNC_ACT_ACTIVATED   0x01
#define ESCREG_SYNC_SYNC0_EN        0x02
#define ESCREG_SYNC_SYNC1_EN        0x04
#define ESCREG_SYNC_AUTO_ACTIVATED  0x08
#define ESCREG_SYNC0_CYCLE_TIME     0x09A0
#define ESCREG_SYNC1_CYCLE_TIME     0x09A4
#define ESCREG_SMENABLE_BIT         0x01
#define ESCREG_AL_STATEMASK         0x001f
#define ESCREG_AL_ALLBUTINITMASK    0x0e
#define ESCREG_AL_ERRACKMASK        0x0f

#define SYNCTYPE_SUPPORT_FREERUN    0x01
#define SYNCTYPE_SUPPORT_SYNCHRON   0x02
#define SYNCTYPE_SUPPORT_DCSYNC0    0x04
#define SYNCTYPE_SUPPORT_DCSYNC1    0x08
#define SYNCTYPE_SUPPORT_SUBCYCLE   0x10

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
#define ALERR_NOVALIDFIRMWARE       0x0014
#define ALERR_INVALIDBOOTMBXCONFIG  0x0015
#define ALERR_INVALIDMBXCONFIG      0x0016
#define ALERR_INVALIDSMCONFIG       0x0017
#define ALERR_SYNCERROR             0x001A
#define ALERR_WATCHDOG              0x001B
#define ALERR_INVALIDOUTPUTSM       0x001D
#define ALERR_INVALIDINPUTSM        0x001E
#define ALERR_DCINVALIDSYNCCFG      0x0030
#define ALERR_DCSYNC0CYCLETIME      0x0036

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
#define COE_MINIMUM_LENGTH              8

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

// Attention! this struct is always little-endian
typedef struct CC_PACKED
{
   uint16_t PSA;
   uint16_t Length;

#if defined(EC_LITTLE_ENDIAN)
   uint8_t Mode:2;
   uint8_t Direction:2;
   uint8_t IntECAT:1;
   uint8_t IntPDI:1;
   uint8_t WTE:1;
   uint8_t R1:1;

   uint8_t IntW:1;
   uint8_t IntR:1;
   uint8_t R2:1;
   uint8_t MBXstat:1;
   uint8_t BUFstat:2;
   uint8_t R3:2;

   uint8_t ECsm:1;
   uint8_t ECrep:1;
   uint8_t ECr4:4;
   uint8_t EClatchEC:1;
   uint8_t EClatchPDI:1;

   uint8_t PDIsm:1;
   uint8_t PDIrep:1;
   uint8_t PDIr5:6;
#endif

#if defined(EC_BIG_ENDIAN)
   uint8_t R1:1;
   uint8_t WTE:1;
   uint8_t IntPDI:1;
   uint8_t IntECAT:1;
   uint8_t Direction:2;
   uint8_t Mode:2;

   uint8_t R3:2;
   uint8_t BUFstat:2;
   uint8_t MBXstat:1;
   uint8_t R2:1;
   uint8_t IntR:1;
   uint8_t IntW:1;

   uint8_t EClatchPDI:1;
   uint8_t EClatchEC:1;
   uint8_t ECr4:4;
   uint8_t ECrep:1;
   uint8_t ECsm:1;

   uint8_t PDIr5:6;
   uint8_t PDIrep:1;
   uint8_t PDIsm:1;
#endif
} _ESCsm;

/* Attention! this struct is always little-endian */
typedef struct CC_PACKED
{
   uint16_t PSA;
   uint16_t Length;
   uint8_t Command;
   uint8_t Status;
   uint8_t ActESC;
   uint8_t ActPDI;
} _ESCsm2;

typedef struct CC_PACKED
{
   uint16_t PSA;
   uint16_t Length;
   uint8_t Command;
} _ESCsmCompact;

typedef struct CC_PACKED
{
   uint16_t ALevent;
   uint16_t ALstatus;
   uint16_t ALcontrol;
   uint16_t ALerror;
   uint16_t DLstatus;
   uint16_t address;
   uint8_t mbxcnt;
   uint8_t mbxincnt;
   uint8_t mbxoutpost;
   uint8_t mbxbackup;
   uint8_t xoe;
   uint8_t txcue;
   uint8_t mbxfree;
   uint8_t segmented;
   void *data;
   uint16_t entries;
   uint16_t frags;
   uint16_t fragsleft;

#if defined(EC_LITTLE_ENDIAN)
   uint8_t r1:1;
   uint8_t toggle:1;
   uint8_t r2:6;
#endif

#if defined(EC_BIG_ENDIAN)
   uint8_t r2:6;
   uint8_t toggle:1;
   uint8_t r1:1;
#endif

   uint8_t SMtestresult;
   int16_t temp;
   uint16_t wdcnt;
   uint32_t PrevTime;
   uint32_t Time;
   _ESCsm SM[4];
} _ESCvar;

typedef struct CC_PACKED
{
   uint16_t length;
   uint16_t address;

#if defined(EC_LITTLE_ENDIAN)
   uint8_t channel:6;
   uint8_t priority:2;

   uint8_t mbxtype:4;
   uint8_t mbxcnt:4;
#endif

#if defined(EC_BIG_ENDIAN)
   uint8_t priority:2;
   uint8_t channel:6;

   uint8_t mbxcnt:4;
   uint8_t mbxtype:4;
#endif
} _MBXh;

typedef struct CC_PACKED
{
   uint16_t numberservice;
} _COEh;

typedef struct CC_PACKED
{
#if defined(EC_LITTLE_ENDIAN)
   uint8_t opcode:7;
   uint8_t incomplete:1;
#endif

#if defined(EC_BIG_ENDIAN)
   uint8_t incomplete:1;
   uint8_t opcode:7;
#endif

   uint8_t reserved;
   uint16_t fragmentsleft;
} _INFOh;

typedef struct CC_PACKED
{
   _MBXh mbxheader;
   uint16_t type;
   uint16_t detail;
} _MBXerr;

typedef struct CC_PACKED
{
   _MBXh mbxheader;
   _COEh coeheader;
   uint8_t command;
   uint16_t index;
   uint8_t subindex;
   uint32_t size;
} _COEsdo;

typedef struct CC_PACKED
{
   _MBXh mbxheader;
   _COEh coeheader;
   _INFOh infoheader;
   uint16_t index;
   uint16_t datatype;
   uint8_t maxsub;
   uint8_t objectcode;
   char name;
} _COEobjdesc;

typedef struct CC_PACKED
{
   _MBXh mbxheader;
   _COEh coeheader;
   _INFOh infoheader;
   uint16_t index;
   uint8_t subindex;
   uint8_t valueinfo;
   uint16_t datatype;
   uint16_t bitlength;
   uint16_t access;
   char name;
} _COEentdesc;

typedef struct CC_PACKED
{
   uint8_t opcode;
   uint8_t reserved;
   union
   {
      uint32_t password;
      uint32_t packetnumber;
      uint32_t errorcode;
   };
} _FOEh;

typedef struct CC_PACKED
{
   _MBXh mbxheader;
   _FOEh foeheader;
   union
   {
      char filename;
      uint8_t data;
      char errortext;
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
   uint8_t state;
} _MBXcontrol;

typedef struct sm_cfg
{
   unsigned int cfg_sma;
   unsigned int cfg_sml;
   unsigned int cfg_sme;
   unsigned int cfg_smc;
   unsigned int cfg_smact;
}sm_cfg_t;

typedef struct esc_cfg
{
   void * user_arg;
   int use_interrupt;
   int watchdog_cnt;
   size_t mbxsize;
   size_t mbxsizeboot;
   int mbxbuffers;
   sm_cfg_t mb[2];
   sm_cfg_t mb_boot[2];
   sm_cfg_t pdosm[2];
   void (*pre_state_change_hook) (uint8_t * as, uint8_t * an);
   void (*post_state_change_hook) (uint8_t * as, uint8_t * an);
   void (*application_hook) (void);
   void (*safeoutput_override) (void);
   int (*pre_object_download_hook) (uint16_t index, uint8_t subindex);
   void (*post_object_download_hook) (uint16_t index, uint8_t subindex);
   void (*rxpdo_override) (void);
   void (*txpdo_override) (void);
   void (*esc_hw_interrupt_enable) (uint32_t mask);
   void (*esc_hw_interrupt_disable) (uint32_t mask);
   void (*esc_hw_eep_handler) (void);
} esc_cfg_t;

/* Stack reference to application configuration of the ESC */
extern esc_cfg_t * esc_cfg;
extern size_t ESC_MBXSIZE;
extern unsigned int ESC_MBX0_sma;
extern unsigned int ESC_MBX0_sml;
extern unsigned int ESC_MBX0_sme;
extern unsigned int ESC_MBX0_smc;
extern unsigned int ESC_MBX1_sma;
extern unsigned int ESC_MBX1_sml;
extern unsigned int ESC_MBX1_sme;
extern unsigned int ESC_MBX1_smc;

#define ESC_MBXBUFFERS      (esc_cfg->mbxbuffers)

#define ESC_MBXSIZE_s       (esc_cfg->mbxsize)
#define ESC_MBX0_sma_s      (esc_cfg->mb[0].cfg_sma)
#define ESC_MBX0_sml_s      (esc_cfg->mb[0].cfg_sml)
#define ESC_MBX0_sme_s      (esc_cfg->mb[0].cfg_sme)
#define ESC_MBX0_smc_s      (esc_cfg->mb[0].cfg_smc)
#define ESC_MBX1_sma_s      (esc_cfg->mb[1].cfg_sma)
#define ESC_MBX1_sml_s      (esc_cfg->mb[1].cfg_sml)
#define ESC_MBX1_sme_s      (esc_cfg->mb[1].cfg_sme)
#define ESC_MBX1_smc_s      (esc_cfg->mb[1].cfg_smc)
#define ESC_MBXSIZE_b       (esc_cfg->mbxsizeboot)
#define ESC_MBX0_sma_b      (esc_cfg->mb_boot[0].cfg_sma)
#define ESC_MBX0_sml_b      (esc_cfg->mb_boot[0].cfg_sml)
#define ESC_MBX0_sme_b      (esc_cfg->mb_boot[0].cfg_sme)
#define ESC_MBX0_smc_b      (esc_cfg->mb_boot[0].cfg_smc)
#define ESC_MBX1_sma_b      (esc_cfg->mb_boot[1].cfg_sma)
#define ESC_MBX1_sml_b      (esc_cfg->mb_boot[1].cfg_sml)
#define ESC_MBX1_sme_b      (esc_cfg->mb_boot[1].cfg_sme)
#define ESC_MBX1_smc_b      (esc_cfg->mb_boot[1].cfg_smc)
#define ESC_SM2_sma         (esc_cfg->pdosm[0].cfg_sma)
#define ESC_SM2_smc         (esc_cfg->pdosm[0].cfg_smc)
#define ESC_SM2_act         (esc_cfg->pdosm[0].cfg_smact)
#define ESC_SM3_sma         (esc_cfg->pdosm[1].cfg_sma)
#define ESC_SM3_smc         (esc_cfg->pdosm[1].cfg_smc)
#define ESC_SM3_act         (esc_cfg->pdosm[1].cfg_smact)

#define ESC_MBXHSIZE        sizeof(_MBXh)
#define ESC_MBXDSIZE        (ESC_MBXSIZE - ESC_MBXHSIZE)
#define ESC_FOEHSIZE        sizeof(_FOEh)
#define ESC_FOE_DATA_SIZE   (ESC_MBXSIZE - (ESC_MBXHSIZE +ESC_FOEHSIZE))

void ESC_config (esc_cfg_t * cfg);
void ESC_ALerror (uint16_t errornumber);
void ESC_ALeventwrite (uint32_t event);
uint32_t ESC_ALeventread (void);
void ESC_ALeventmaskwrite (uint32_t mask);
uint32_t ESC_ALeventmaskread (void);
void ESC_ALstatus (uint8_t status);
void ESC_SMstatus (uint8_t n);
uint8_t ESC_WDstatus (void);
uint8_t ESC_claimbuffer (void);
uint8_t ESC_startmbx (uint8_t state);
void ESC_stopmbx (void);
void MBX_error (uint16_t error);
uint8_t ESC_mbxprocess (void);
void ESC_xoeprocess (void);
uint8_t ESC_startinput (uint8_t state);
void ESC_stopinput (void);
uint8_t ESC_startoutput (uint8_t state);
void ESC_stopoutput (void);
void ESC_state (void);
void ESC_sm_act_event (void);

/* From hardware file */
void ESC_read (uint16_t address, void *buf, uint16_t len);
void ESC_write (uint16_t address, void *buf, uint16_t len);
void ESC_init (const esc_cfg_t * cfg);
void ESC_reset (void);
void ESC_interrupt_enable (uint32_t mask);
void ESC_interrupt_disable (uint32_t mask);

/* From application */
extern void APP_safeoutput ();
extern volatile _ESCvar ESCvar;
extern uint8_t MBX[];
extern _MBXcontrol MBXcontrol[];
extern uint8_t MBXrun;
extern uint16_t ESC_SM2_sml, ESC_SM3_sml;
extern uint8_t dc_sync;
extern int8_t sync_counter;
extern uint16_t sync_counter_limit;
extern uint16_t TXPDOsize;
extern uint16_t RXPDOsize;

typedef struct
{
   uint8_t state;
} _App;

#define APPSTATE_IDLE      0x00
#define APPSTATE_INPUT     0x01
#define APPSTATE_OUTPUT    0x02

extern _App App;

void DIG_process (uint8_t flags);

#define DIG_PROCESS_INPUTS_FLAG     0x01
#define DIG_PROCESS_OUTPUTS_FLAG    0x02
#define DIG_PROCESS_WD_FLAG         0x04
#define DIG_PROCESS_APP_HOOK_FLAG   0x08

/* ATOMIC operations are used when running interrupt driven */
#ifndef CC_ATOMIC_SET
#define CC_ATOMIC_SET(var,val)   (var = val)
#endif

#ifndef CC_ATOMIC_GET
#define CC_ATOMIC_GET(var)       (var)
#endif

#ifndef CC_ATOMIC_ADD
#define CC_ATOMIC_ADD(var,val)   (var += val)
#endif

#ifndef CC_ATOMIC_SUB
#define CC_ATOMIC_SUB(var,val)   (var -= val)
#endif

#ifndef CC_ATOMIC_AND
#define CC_ATOMIC_AND(var,val)   (var &= val)
#endif

#ifndef CC_ATOMIC_OR
#define CC_ATOMIC_OR(var,val)    (var |= val)
#endif


#endif
