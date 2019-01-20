#ifndef __UTYPES_H__
#define __UTYPES_H__

#include <cc.h>
#include "config.h"

/* NOTE: Not packed, does not use bitfields */

typedef struct
{
   /* Inputs */
   uint8_t IN1; /* (BIT1) */
   uint8_t IN2; /* (BIT2) */
   uint8_t IN3; /* (BIT3) */
   uint8_t IN4; /* (BIT4) */

   /* Outputs */
   struct
   {
      int8_t OUT1;
      int16_t OUT2;
      int32_t OUT3;
   } Outputs;

   /* Parameters */
   int32_t PARAM1;
   int32_t PARAM2;

   /* Manufacturer specific data */
   struct
   {
      uint16_t SyncType;
      uint32_t CycleTime;
      uint32_t ShiftTime;
      uint16_t SyncTypeSupport;
      uint32_t MinCycleTime;
      uint32_t CalcCopyTime;
      uint32_t MinDelayTime;
      uint16_t GetCycleTime;
      uint32_t DelayTime;
      uint32_t Sync0CycleTime;
      uint16_t SMEventMissedCnt;
      uint16_t CycleTimeTooSmallCnt;
      uint16_t ShiftTimeTooSmallCnt;
      uint16_t RxPDOToggleFailed;
      uint32_t MinCycleDist;
      uint32_t MaxCycleDist;
      uint32_t MinSMSYNCDist;
      uint32_t MaxSMSYNCDist;
      uint8_t Dummy_x14;
      uint8_t SyncError;
   } SyncMgrParam;
   
   struct
   {
      uint8_t Dummy_x01;
      uint16_t SyncErrorCounterLimit;
   } ErrorSettings;
   
   /* Storage for configurable PDO:s */
   struct
   {
      uint8_t maxsub;
      uint32_t value[4];
   } PDO[2];

   /* Storage for configurable Sync Managers */
   struct
   {
      uint8_t maxsub;
      uint16_t value[4];
   } SM[2];

} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
