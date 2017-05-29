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
      uint8_t Button1;
   } CC_PACKED Buttons;
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
      uint8_t LED0;
   } CC_PACKED LEDgroup0;
   CC_PACKED_END
   CC_PACKED_BEGIN
   struct
   {
      uint8_t LED1;
   } CC_PACKED LEDgroup1;
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
      uint8_t Reserved0x18;
      uint8_t Reserved0x1C;
      uint8_t Reserved0x1D;
      uint8_t Reserved0x1E;
      uint8_t Reserved0x1F;
      uint8_t SyncError;
   } CC_PACKED SyncMgrParam;
   CC_PACKED_END
} CC_PACKED _Mbuffer;
CC_PACKED_END

extern _Rbuffer Rb;
extern _Wbuffer Wb;
extern _Cbuffer Cb;
extern _Mbuffer Mb;

#endif /* __UTYPES_H__ */
