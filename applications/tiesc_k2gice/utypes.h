#ifndef __UTYPES_H__
#define __UTYPES_H__

#include <cc.h>

/* Inputs */
CC_PACKED_BEGIN
typedef struct
{
   uint32_t BUTTON;
} CC_PACKED _Rbuffer;
CC_PACKED_END

/* Outputs */
CC_PACKED_BEGIN
typedef struct
{
   uint32_t LED;
} CC_PACKED _Wbuffer;
CC_PACKED_END

/* Parameters */
CC_PACKED_BEGIN
typedef struct
{
} CC_PACKED _Cbuffer;
CC_PACKED_END

/* Manufacturer specific data */
CC_PACKED_BEGIN
typedef struct
{
} CC_PACKED _Mbuffer;
CC_PACKED_END

extern _Rbuffer Rb;
extern _Wbuffer Wb;
extern _Cbuffer Cb;
extern _Mbuffer Mb;

#endif /* __UTYPES_H__ */
