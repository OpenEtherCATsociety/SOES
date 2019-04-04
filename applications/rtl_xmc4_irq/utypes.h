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
   struct
   {
      uint32_t Local_Error_Reaction;
      uint16_t SyncErrorCounterLimit;
   } ErrorSettings;

   struct
   {
      uint16_t Sync_mode;
      uint32_t CycleTime;
      uint32_t ShiftTime;
      uint16_t Sync_modes_supported;
      uint32_t Minimum_Cycle_Time;
      uint32_t Calc_and_Copy_Time;
      uint32_t Minimum_Delay_Time;
      uint16_t GetCycleTime;
      uint32_t DelayTime;
      uint32_t Sync0CycleTime;
      uint16_t SMEventMissedCnt;
      uint16_t CycleTimeTooSmallCnt;
      uint16_t Shift_too_short_counter;
      uint16_t RxPDOToggleFailed;
      uint32_t Minimum_Cycle_Distance;
      uint32_t Maximum_Cycle_Distance;
      uint32_t Minimum_SM_Sync_Distance;
      uint32_t Maximum_SM_Sync_Distance;
      uint8_t SyncError;
   } SyncMgrParam;


   /* Dynamic TX PDO:s */

   /* Dynamic RX PDO:s */

   /* Sync Managers */

} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
