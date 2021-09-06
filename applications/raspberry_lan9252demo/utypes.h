#ifndef __UTYPES_H__
#define __UTYPES_H__

#include "cc.h"

/* Object dictionary storage */

typedef struct
{
   /* Identity */

   uint32_t serial;

   /* Inputs */

   struct
   {
      uint8_t Button0;
      uint8_t Button1;
      uint8_t Button2;
      uint8_t Button3;
      uint8_t Button4;
      uint8_t Button5;
   } Buttons;

   /* Outputs */

   struct
   {
      uint8_t LED0;
      uint8_t LED1;
      uint8_t LED2;
      uint8_t LED3;
      uint8_t LED4;
      uint8_t LED5;
   } LEDs;

   /* Parameters */

   struct
   {
      uint32_t Multiplier;
   } Parameters;

} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
