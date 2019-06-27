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

CC_PACKED_BEGIN
typedef struct CC_PACKED
{
   uint16_t subindex;
   uint16_t datatype;
   uint16_t bitlength;
   uint16_t flags;
   const char *name;
   uint32_t value;
   void *data;
} _objd;
CC_PACKED_END

CC_PACKED_BEGIN
typedef struct CC_PACKED
{
   uint16_t index;
   uint16_t objtype;
   uint8_t maxsub;
   uint8_t pad1;
   const char *name;
   const _objd *objdesc;
} _objectlist;
CC_PACKED_END

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
#define DTYPE_BIT1              0x0030
#define DTYPE_BIT2              0x0031
#define DTYPE_BIT3              0x0032
#define DTYPE_BIT4              0x0033
#define DTYPE_BIT5              0x0034
#define DTYPE_BIT6              0x0035
#define DTYPE_BIT7              0x0036
#define DTYPE_BIT8              0x0037

#define ATYPE_Rpre              0x01
#define ATYPE_Rsafe             0x02
#define ATYPE_Rop               0x04
#define ATYPE_Wpre              0x08
#define ATYPE_Wsafe             0x10
#define ATYPE_Wop               0x20
#define ATYPE_RXPDO             0x40
#define ATYPE_TXPDO             0x80

#define ATYPE_RO                (ATYPE_Rpre | ATYPE_Rsafe | ATYPE_Rop)
#define ATYPE_RW                (ATYPE_Wpre | ATYPE_Wsafe | ATYPE_Wop | ATYPE_RO)
#define ATYPE_RWpre             (ATYPE_Wpre | ATYPE_RO)

#define TX_PDO_OBJIDX           0x1c13
#define RX_PDO_OBJIDX           0x1c12

void ESC_coeprocess (void);
uint16_t sizeOfPDO (uint16_t index, int * nmappings, _SMmap * sm, int max_mappings);
void SDO_abort (uint16_t index, uint8_t subindex, uint32_t abortcode);
void COE_initDefaultValues (void);

void COE_pdoPack (uint8_t * buffer, int nmappings, _SMmap * sm);
void COE_pdoUnpack (uint8_t * buffer, int nmappings, _SMmap * sm);
uint8_t COE_maxSub (uint16_t index);

extern void ESC_objecthandler (uint16_t index, uint8_t subindex, uint16_t flags);
extern uint32_t ESC_pre_objecthandler (uint16_t index,
      uint8_t subindex,
      void * data,
      size_t size,
      uint16_t flags);
extern const _objectlist SDOobjects[];

#endif
