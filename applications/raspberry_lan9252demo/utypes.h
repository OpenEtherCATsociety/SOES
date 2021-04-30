#ifndef __UTYPES_H__
#define __UTYPES_H__

#include "cc.h"


#define DEVICE_EASYCAT
//#define DEVICE_ETHERC

#ifdef DEVICE_EASYCAT
#define DEVICE_NAME "EasyCAT 32+32 rev 1"
#else
#ifdef DEVICE_ETHERC
#define DEVICE_NAME "EtherBerry 32+32"
#else
#define DEVICE_NAME "Unknown"
#endif
#endif

#define BYTE_NUM 32 // or 16, 64, 128


/* Object dictionary storage */

typedef struct
{
   /* Inputs */

   /* Outputs */

   /* Parameters */

   /* Manufacturer specific data */

   /* Dynamic TX PDO:s */
   uint8_t txpdo[BYTE_NUM];

   /* Dynamic RX PDO:s */
   uint8_t rxpdo[BYTE_NUM];
   
   /* Sync Managers */

} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
