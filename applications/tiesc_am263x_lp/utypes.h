#ifndef __UTYPES_H__
#define __UTYPES_H__

#include "cc.h"


/* Object dictionary storage */

typedef struct
{
   /* Inputs */
   uint32_t BUTTON;

   /* Outputs */
   uint32_t LED;

   /* Parameters */

   /* Manufacturer specific data */

   /* Dynamic TX PDO:s */

   /* Dynamic RX PDO:s */

   /* Sync Managers */

} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
