#ifndef __UTYPES_H__
#define __UTYPES_H__

#include <cc.h>

/* Inputs */
CC_PACKED_BEGIN
typedef struct
{
   CC_PACKED_BEGIN   
   struct
   {
      uint8_t B;
   } CC_PACKED Button1;
   CC_PACKED_END
   CC_PACKED_BEGIN   
   struct
   {
      uint32_t B;
   } CC_PACKED Button2;
   CC_PACKED_END
} CC_PACKED _Rbuffer;
CC_PACKED_END

/* Outputs */
CC_PACKED_BEGIN
typedef struct
{
   CC_PACKED_BEGIN
   struct
   {
      uint32_t LED;
   } CC_PACKED LEDgroup1;
   CC_PACKED_END
   CC_PACKED_BEGIN
   struct
   {
      uint8_t LED;
   } CC_PACKED LEDgroup2;
   CC_PACKED_END
   CC_PACKED_BEGIN
   struct
   {
      uint8_t LED;
   } CC_PACKED LEDgroup3;
   CC_PACKED_END
   CC_PACKED_BEGIN
   struct
   {
      uint8_t LED;
   } CC_PACKED LEDgroup4;
   CC_PACKED_END
   CC_PACKED_BEGIN
   struct
   {
      uint8_t LED5;
      uint8_t LED678;
   } CC_PACKED LEDgroup5;
   CC_PACKED_END
} CC_PACKED _Wbuffer;
CC_PACKED_END

/* Parameters */
CC_PACKED_BEGIN
typedef struct
{
   CC_PACKED_BEGIN
   struct
   {
      uint32_t Multiplier;
   } CC_PACKED Parameters;
   CC_PACKED_END
   uint32_t variableRW;
} CC_PACKED _Cbuffer;
CC_PACKED_END

/* Manufacturer specific data */
CC_PACKED_BEGIN
typedef struct
{
   CC_PACKED_BEGIN
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
   } CC_PACKED SyncMgrParam;
   CC_PACKED_END
   CC_PACKED_BEGIN
   struct
   {
      uint8_t Dummy_x01;
      uint16_t SyncErrorCounterLimit;
   } CC_PACKED ErrorSettings;
   CC_PACKED_END
} CC_PACKED _Mbuffer;
CC_PACKED_END

extern _Rbuffer Rb;
extern _Wbuffer Wb;
extern _Cbuffer Cb;
extern _Mbuffer Mb;

#endif /* __UTYPES_H__ */
