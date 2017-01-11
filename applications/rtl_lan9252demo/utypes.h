#ifndef __UTYPES_H__
#define __UTYPES_H__

#include <cc.h>

CC_PACKED_BEGIN
typedef struct
{
   CC_PACKED_BEGIN   
   struct
   {
      uint8_t Button1;
   } CC_PACKED Buttons;
   CC_PACKED_END
} CC_PACKED _Rbuffer;
CC_PACKED_END

CC_PACKED_BEGIN
typedef struct
{
   CC_PACKED_BEGIN
   struct
   {
      uint8_t LED0;
      uint8_t LED1;
   } CC_PACKED LEDs;
   CC_PACKED_END
} CC_PACKED _Wbuffer;
CC_PACKED_END

CC_PACKED_BEGIN
typedef struct
{
   CC_PACKED_BEGIN
   struct
   {
      uint32_t Multiplier;
   } CC_PACKED Parameters;
   CC_PACKED_END
} CC_PACKED _Cbuffer;
CC_PACKED_END

extern _Rbuffer Rb;
extern _Wbuffer Wb;
extern _Cbuffer Cb;

#endif /* __UTYPES_H__ */
