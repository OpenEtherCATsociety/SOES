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
      uint8_t LED1;
   } LEDs;


   /* Parameters */
   struct
   {
      uint32_t Multiplier;
   } Parameters;


   /* Manufacturer specific data */

   /* Dynamic TX PDO:s */

   /* Dynamic RX PDO:s */

   /* Sync Managers */

} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
