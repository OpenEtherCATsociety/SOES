#ifndef __UTYPES_H__
#define __UTYPES_H__

#include "cc.h"

/* Object dictionary storage */

typedef struct
{
   /* Identity */
   uint32_t serial;

#ifdef USE_MDP
   /* MDP Module data */
   struct
   {
      /* Module 0: Digital I/O Module */
      uint8_t module0_input[8];   /* 8 digital inputs */
      uint8_t module0_output[8]; /* 8 digital outputs */
      
      /* Module 1: Analog Input Module */
      uint16_t module1_input[4];  /* 4 analog inputs */
      
      /* Module 2: Analog Output Module */
      uint16_t module2_output[4]; /* 4 analog outputs */
   } Mdp;
#endif

} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
