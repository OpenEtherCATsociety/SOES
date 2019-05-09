#ifndef __UTYPES_H__
#define __UTYPES_H__

#include "cc.h"


/* Object dictionary storage */

typedef struct
{
   /* Inputs */
   struct
   {
      uint8_t Button1;
   } Buttons;


   /* Outputs */
   struct
   {
      uint8_t LED0;
   } LEDgroup0;

   struct
   {
      uint8_t LED1;
   } LEDgroup1;


   /* Parameters */
   struct
   {
      uint32_t Multiplier;
   } Parameters;

   uint32_t variableRW;

   /* Manufacturer specific data */

   /* Dynamic TX PDO:s */

   /* Dynamic RX PDO:s */

   /* Sync Managers */

} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
