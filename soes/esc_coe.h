/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * Headerfile for esc_coe.c
 */

#ifndef __esc_coe__
#define __esc_coe__

#include <cc.h>


typedef struct
{
   uint16_t subindex;
   uint16_t datatype;
   uint16_t bitlength;
   uint16_t flags;
   const char *name;
   uint32_t value;
   void *data;
} _objd;


typedef struct
{
   uint16_t index;
   uint16_t objtype;
   uint8_t maxsub;
   uint8_t pad1;
   const char *name;
   const _objd *objdesc;
} _objectlist;


typedef struct
{
   const _objd * obj;
   uint16_t offset;
} _SMmap;

#define OBJH_READ               0
#define OBJH_WRITE              1

#define OTYPE_DOMAIN            0x0002
#define OTYPE_DEFTYPE           0x0005
#define OTYPE_DEFSTRUCT         0x0006
#define OTYPE_VAR               0x0007
#define OTYPE_ARRAY             0x0008
#define OTYPE_RECORD            0x0009

#define DTYPE_BOOLEAN           0x0001
#define DTYPE_INTEGER8          0x0002
#define DTYPE_INTEGER16         0x0003
#define DTYPE_INTEGER32         0x0004
#define DTYPE_UNSIGNED8         0x0005
#define DTYPE_UNSIGNED16        0x0006
#define DTYPE_UNSIGNED32        0x0007
#define DTYPE_REAL32            0x0008
#define DTYPE_VISIBLE_STRING    0x0009
#define DTYPE_OCTET_STRING      0x000A
#define DTYPE_UNICODE_STRING    0x000B
#define DTYPE_INTEGER24         0x0010
#define DTYPE_UNSIGNED24        0x0016
#define DTYPE_INTEGER64         0x0015
#define DTYPE_UNSIGNED64        0x001B
#define DTYPE_REAL64            0x0011
#define DTYPE_PDO_MAPPING       0x0021
#define DTYPE_IDENTITY          0x0023
#define DTYPE_BITARR8           0x002D
#define DTYPE_BITARR16          0x002E
#define DTYPE_BITARR32          0x002F
#define DTYPE_BIT1              0x0030
#define DTYPE_BIT2              0x0031
#define DTYPE_BIT3              0x0032
#define DTYPE_BIT4              0x0033
#define DTYPE_BIT5              0x0034
#define DTYPE_BIT6              0x0035
#define DTYPE_BIT7              0x0036
#define DTYPE_BIT8              0x0037
#define DTYPE_ARRAY_OF_INT      0x0260
#define DTYPE_ARRAY_OF_SINT     0x0261
#define DTYPE_ARRAY_OF_DINT     0x0262
#define DTYPE_ARRAY_OF_UDINT    0x0263

#define ATYPE_Rpre              0x01
#define ATYPE_Rsafe             0x02
#define ATYPE_Rop               0x04
#define ATYPE_Wpre              0x08
#define ATYPE_Wsafe             0x10
#define ATYPE_Wop               0x20
#define ATYPE_RXPDO             0x40
#define ATYPE_TXPDO             0x80
#define ATYPE_BACKUP            0x100
#define ATYPE_SETTING           0x200

#define ATYPE_RO                (ATYPE_Rpre | ATYPE_Rsafe | ATYPE_Rop)
#define ATYPE_WO                (ATYPE_Wpre | ATYPE_Wsafe | ATYPE_Wop)
#define ATYPE_RW                (ATYPE_RO | ATYPE_WO)
#define ATYPE_RWpre             (ATYPE_Wpre | ATYPE_RO)
#define ATYPE_RWop              (ATYPE_Wop | ATYPE_RO)
#define ATYPE_RWpre_safe        (ATYPE_Wpre | ATYPE_Wsafe | ATYPE_RO)

#define TX_PDO_OBJIDX           0x1c13
#define RX_PDO_OBJIDX           0x1c12

#define COMPLETE_ACCESS_FLAG    (1 << 15)

void ESC_coeprocess (void);
int16_t SDO_findsubindex (int16_t nidx, uint8_t subindex);
int32_t SDO_findobject (uint16_t index);
uint16_t sizeOfPDO (uint16_t index, int * nmappings, _SMmap * sm, int max_mappings);
void COE_initDefaultValues (void);

void COE_pdoPack (uint8_t * buffer, int nmappings, _SMmap * sm);
void COE_pdoUnpack (uint8_t * buffer, int nmappings, _SMmap * sm);
uint8_t COE_maxSub (uint16_t index);

extern uint32_t ESC_download_post_objecthandler (uint16_t index, uint8_t subindex, uint16_t flags);
extern uint32_t ESC_download_pre_objecthandler (uint16_t index,
      uint8_t subindex,
      void * data,
      size_t size,
      uint16_t flags);
extern uint32_t ESC_upload_pre_objecthandler (uint16_t index,
      uint8_t subindex,
      void * data,
      size_t *size,
      uint16_t flags);
extern uint32_t ESC_upload_post_objecthandler (uint16_t index, uint8_t subindex, uint16_t flags);
extern const _objectlist SDOobjects[];

#endif
