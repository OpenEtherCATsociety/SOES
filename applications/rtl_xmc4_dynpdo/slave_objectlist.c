#ifndef SOES_V1
#include "esc.h"
#include "esc_coe.h"
#include "utypes.h"
#include <stddef.h>

#ifndef HW_REV
#define HW_REV "1.0"
#endif

#ifndef SW_REV
#define SW_REV "1.0"
#endif

static const char acName1000[] = "Device Type";
static const char acName1000_0[] = "Device Type";
static const char acName1008[] = "Device Name";
static const char acName1008_0[] = "Device Name";
static const char acName1009[] = "Hardware Version";
static const char acName1009_0[] = "Hardware Version";
static const char acName100A[] = "Software Version";
static const char acName100A_0[] = "Software Version";
static const char acName1018[] = "Identity Object";
static const char acName1018_00[] = "Max SubIndex";
static const char acName1018_01[] = "Vendor ID";
static const char acName1018_02[] = "Product Code";
static const char acName1018_03[] = "Revision Number";
static const char acName1018_04[] = "Serial Number";
static const char acName10F1[] = "ErrorSettings";
static const char acName10F1_00[] = "Max SubIndex";
static const char acName10F1_01[] = "Dummy_x01";
static const char acName10F1_02[] = "SyncErrorCounterLimit";
static const char acName1600[] = "Outputs";
static const char acName1600_00[] = "Max SubIndex";
static const char acName1600_01[] = "OUT1";
static const char acName1600_02[] = "OUT2";
static const char acName1600_03[] = "OUT3";
static const char acName1A00[] = "IN1";
static const char acName1A00_00[] = "Max SubIndex";
static const char acName1A00_01[] = "IN1";
static const char acName1A00_02[] = "Padding";
static const char acName1A01[] = "IN2";
static const char acName1A01_00[] = "Max SubIndex";
static const char acName1A01_01[] = "IN2";
static const char acName1A01_02[] = "Padding";
static const char acName1A02[] = "IN3";
static const char acName1A02_00[] = "Max SubIndex";
static const char acName1A02_01[] = "IN3";
static const char acName1A02_02[] = "Padding";
static const char acName1A03[] = "IN4";
static const char acName1A03_00[] = "Max SubIndex";
static const char acName1A03_01[] = "IN4";
static const char acName1A03_02[] = "Padding";
static const char acName1C00[] = "Sync Manager Communication Type";
static const char acName1C00_00[] = "Max SubIndex";
static const char acName1C00_01[] = "Communications Type SM0";
static const char acName1C00_02[] = "Communications Type SM1";
static const char acName1C00_03[] = "Communications Type SM2";
static const char acName1C00_04[] = "Communications Type SM3";
static const char acName1C12[] = "Sync Manager 2 PDO Assignment";
static const char acName1C12_00[] = "Max SubIndex";
static const char acName1C12_01[] = "PDO Mapping";
static const char acName1C12_02[] = "PDO Mapping";
static const char acName1C12_03[] = "PDO Mapping";
static const char acName1C12_04[] = "PDO Mapping";
static const char acName1C13[] = "Sync Manager 3 PDO Assignment";
static const char acName1C13_00[] = "Max SubIndex";
static const char acName1C13_01[] = "PDO Mapping";
static const char acName1C13_02[] = "PDO Mapping";
static const char acName1C13_03[] = "PDO Mapping";
static const char acName1C13_04[] = "PDO Mapping";
static const char acName1C32[] = "SyncMgrParam";
static const char acName1C32_00[] = "Max SubIndex";
static const char acName1C32_01[] = "SyncType";
static const char acName1C32_02[] = "CycleTime";
static const char acName1C32_03[] = "ShiftTime";
static const char acName1C32_04[] = "SyncTypeSupport";
static const char acName1C32_05[] = "MinCycleTime";
static const char acName1C32_06[] = "CalcCopyTime";
static const char acName1C32_07[] = "MinDelayTime";
static const char acName1C32_08[] = "GetCycleTime";
static const char acName1C32_09[] = "DelayTime";
static const char acName1C32_0A[] = "Sync0CycleTime";
static const char acName1C32_0B[] = "SMEventMissedCnt";
static const char acName1C32_0C[] = "CycleTimeTooSmallCnt";
static const char acName1C32_0D[] = "ShiftTimeTooSmallCnt";
static const char acName1C32_0E[] = "RxPDOToggleFailed";
static const char acName1C32_0F[] = "MinCycleDist";
static const char acName1C32_10[] = "MaxCycleDist";
static const char acName1C32_11[] = "MinSMSYNCDist";
static const char acName1C32_12[] = "MaxSMSYNCDist";
static const char acName1C32_14[] = "Dummy_x14";
static const char acName1C32_20[] = "SyncError";
static const char acName6000[] = "IN1";
static const char acName6000_0[] = "IN1";
static const char acName6001[] = "IN2";
static const char acName6001_0[] = "IN2";
static const char acName6002[] = "IN3";
static const char acName6002_0[] = "IN3";
static const char acName6003[] = "IN4";
static const char acName6003_0[] = "IN4";
static const char acName7000[] = "Outputs";
static const char acName7000_00[] = "Max SubIndex";
static const char acName7000_01[] = "OUT1";
static const char acName7000_02[] = "OUT2";
static const char acName7000_03[] = "OUT3";
static const char acName8000[] = "PARAM1";
static const char acName8000_0[] = "PARAM1";
static const char acName8001[] = "PARAM2";
static const char acName8001_0[] = "PARAM2";

const _objd SDO1000[] =
{
  {0x0, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1000_0, 0x00001389, NULL},
};
const _objd SDO1008[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 40, ATYPE_RO, acName1008_0, 0, "slave"},
};
const _objd SDO1009[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 24, ATYPE_RO, acName1009_0, 0, HW_REV},
};
const _objd SDO100A[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 24, ATYPE_RO, acName100A_0, 0, SW_REV},
};
const _objd SDO1018[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1018_00, 4, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_01, 0x1337, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_02, 0x10cc, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_03, 0, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_04, 0x00000000, NULL},
};
const _objd SDO10F1[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName10F1_00, 2, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName10F1_01, 0, &Obj.ErrorSettings.Dummy_x01},
  {0x02, DTYPE_UNSIGNED16, 16, ATYPE_RW, acName10F1_02, 2, &Obj.ErrorSettings.SyncErrorCounterLimit},
};
const _objd SDO1600[] =
{
  /* Writeable PDO */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RWpre, acName1600_00, 3, &Obj.PDO[0].maxsub},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RWpre, acName1600_01, 0x70000108, &Obj.PDO[0].value[0]},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RWpre, acName1600_02, 0x70000210, &Obj.PDO[0].value[1]},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RWpre, acName1600_03, 0x70000320, &Obj.PDO[0].value[2]},
};
const _objd SDO1A00[] =
{
  /* Writeable PDO */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RWpre, acName1A00_00, 2, &Obj.PDO[1].maxsub},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RWpre, acName1A00_01, 0x60000001, &Obj.PDO[1].value[0]},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RWpre, acName1A00_02, 0x00000007, &Obj.PDO[1].value[1]},
};
const _objd SDO1A01[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A01_00, 2, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A01_01, 0x60010002, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A01_02, 0x00000006, NULL},
};
const _objd SDO1A02[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A02_00, 2, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A02_01, 0x60020003, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A02_02, 0x00000005, NULL},
};
const _objd SDO1A03[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A03_00, 2, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A03_01, 0x60030004, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A03_02, 0x00000004, NULL},
};
const _objd SDO1C00[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_00, 4, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_01, 1, NULL},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_02, 2, NULL},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_03, 3, NULL},
  {0x04, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C00_04, 4, NULL},
};
const _objd SDO1C12[] =
{
  /* Writeable SM */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RWpre, acName1C12_00, 1, &Obj.SM[0].maxsub},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RWpre, acName1C12_01, 0x1600, &Obj.SM[0].value[0]},
  {0x02, DTYPE_UNSIGNED16, 16, ATYPE_RWpre, acName1C12_02, 0, &Obj.SM[0].value[1]},
  {0x03, DTYPE_UNSIGNED16, 16, ATYPE_RWpre, acName1C12_03, 0, &Obj.SM[0].value[2]},
  {0x04, DTYPE_UNSIGNED16, 16, ATYPE_RWpre, acName1C12_04, 0, &Obj.SM[0].value[3]},
};
const _objd SDO1C13[] =
{
  /* Writeable SM */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RWpre, acName1C13_00, 4, &Obj.SM[1].maxsub},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RWpre, acName1C13_01, 0x1A00, &Obj.SM[1].value[0]},
  {0x02, DTYPE_UNSIGNED16, 16, ATYPE_RWpre, acName1C13_02, 0x1A01, &Obj.SM[1].value[1]},
  {0x03, DTYPE_UNSIGNED16, 16, ATYPE_RWpre, acName1C13_03, 0x1A02, &Obj.SM[1].value[2]},
  {0x04, DTYPE_UNSIGNED16, 16, ATYPE_RWpre, acName1C13_04, 0x1A03, &Obj.SM[1].value[3]},
};
const _objd SDO1C32[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C32_00, 32, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_01, 1, &Obj.SyncMgrParam.SyncType},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_02, 0, &Obj.SyncMgrParam.CycleTime},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_03, 0, &Obj.SyncMgrParam.ShiftTime},
  {0x04, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_04, 0x6, &Obj.SyncMgrParam.SyncTypeSupport},
  {0x05, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_05, 125000, &Obj.SyncMgrParam.MinCycleTime},
  {0x06, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_06, 0, &Obj.SyncMgrParam.CalcCopyTime},
  {0x07, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_07, 0, &Obj.SyncMgrParam.MinDelayTime},
  {0x08, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_08, 0, &Obj.SyncMgrParam.GetCycleTime},
  {0x09, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_09, 0, &Obj.SyncMgrParam.DelayTime},
  {0x0A, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_0A, 0, &Obj.SyncMgrParam.Sync0CycleTime},
  {0x0B, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_0B, 0, &Obj.SyncMgrParam.SMEventMissedCnt},
  {0x0C, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_0C, 0, &Obj.SyncMgrParam.CycleTimeTooSmallCnt},
  {0x0D, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_0D, 0, &Obj.SyncMgrParam.ShiftTimeTooSmallCnt},
  {0x0E, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_0E, 0, &Obj.SyncMgrParam.RxPDOToggleFailed},
  {0x0F, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_0F, 0, &Obj.SyncMgrParam.MinCycleDist},
  {0x10, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_10, 0, &Obj.SyncMgrParam.MaxCycleDist},
  {0x11, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_11, 0, &Obj.SyncMgrParam.MinSMSYNCDist},
  {0x12, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_12, 0, &Obj.SyncMgrParam.MaxSMSYNCDist},
  {0x14, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C32_14, 0, &Obj.SyncMgrParam.Dummy_x14},
  {0x20, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C32_20, 0, &Obj.SyncMgrParam.SyncError},
};
const _objd SDO6000[] =
{
  {0x0, DTYPE_BIT1, 1, ATYPE_TXPDO | ATYPE_RO, acName6000_0, 0, &Obj.IN1},
};
const _objd SDO6001[] =
{
  {0x0, DTYPE_BIT2, 2, ATYPE_TXPDO | ATYPE_RO, acName6001_0, 0, &Obj.IN2},
};
const _objd SDO6002[] =
{
  {0x0, DTYPE_BIT3, 3, ATYPE_TXPDO | ATYPE_RO, acName6002_0, 0, &Obj.IN3},
};
const _objd SDO6003[] =
{
  {0x0, DTYPE_BIT4, 4, ATYPE_TXPDO | ATYPE_RO, acName6003_0, 0, &Obj.IN4},
};
const _objd SDO7000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName7000_00, 3, NULL},
  {0x01, DTYPE_INTEGER8, 8, ATYPE_RXPDO | ATYPE_RO, acName7000_01, 0, &Obj.Outputs.OUT1},
  {0x02, DTYPE_INTEGER16, 16, ATYPE_RXPDO | ATYPE_RO, acName7000_02, 0, &Obj.Outputs.OUT2},
  {0x03, DTYPE_INTEGER32, 32, ATYPE_RXPDO | ATYPE_RO, acName7000_03, 0, &Obj.Outputs.OUT3},
};
const _objd SDO8000[] =
{
  {0x0, DTYPE_INTEGER32, 32, ATYPE_RW, acName8000_0, 0x42, &Obj.PARAM1},
};
const _objd SDO8001[] =
{
  {0x0, DTYPE_INTEGER32, 32, ATYPE_RW, acName8001_0, 99, &Obj.PARAM2},
};

const _objectlist SDOobjects[] =
{
  {0x1000, OTYPE_VAR, 0, 0, acName1000, SDO1000},
  {0x1008, OTYPE_VAR, 0, 0, acName1008, SDO1008},
  {0x1009, OTYPE_VAR, 0, 0, acName1009, SDO1009},
  {0x100A, OTYPE_VAR, 0, 0, acName100A, SDO100A},
  {0x1018, OTYPE_RECORD, 4, 0, acName1018, SDO1018},
  {0x10F1, OTYPE_RECORD, 2, 0, acName10F1, SDO10F1},
  {0x1600, OTYPE_RECORD, 3, 0, acName1600, SDO1600},
  {0x1A00, OTYPE_RECORD, 2, 0, acName1A00, SDO1A00},
  {0x1A01, OTYPE_RECORD, 2, 0, acName1A01, SDO1A01},
  {0x1A02, OTYPE_RECORD, 2, 0, acName1A02, SDO1A02},
  {0x1A03, OTYPE_RECORD, 2, 0, acName1A03, SDO1A03},
  {0x1C00, OTYPE_ARRAY, 4, 0, acName1C00, SDO1C00},
  {0x1C12, OTYPE_ARRAY, 1, 0, acName1C12, SDO1C12},
  {0x1C13, OTYPE_ARRAY, 4, 0, acName1C13, SDO1C13},
  {0x1C32, OTYPE_RECORD, 32, 0, acName1C32, SDO1C32},
  {0x6000, OTYPE_VAR, 0, 0, acName6000, SDO6000},
  {0x6001, OTYPE_VAR, 0, 0, acName6001, SDO6001},
  {0x6002, OTYPE_VAR, 0, 0, acName6002, SDO6002},
  {0x6003, OTYPE_VAR, 0, 0, acName6003, SDO6003},
  {0x7000, OTYPE_RECORD, 3, 0, acName7000, SDO7000},
  {0x8000, OTYPE_VAR, 0, 0, acName8000, SDO8000},
  {0x8001, OTYPE_VAR, 0, 0, acName8001, SDO8001},
  {0xffff, 0xff, 0xff, 0xff, NULL, NULL}
};
#endif
