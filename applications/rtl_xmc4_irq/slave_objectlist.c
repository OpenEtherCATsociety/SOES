#ifndef SOES_V1
#include "esc_coe.h"
#include "utypes.h"
#include <stddef.h>

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
static const char acName1600[] = "LEDgroup0";
static const char acName1600_00[] = "Max SubIndex";
static const char acName1600_01[] = "LED0";
static const char acName1601[] = "LEDgroup1";
static const char acName1601_00[] = "Max SubIndex";
static const char acName1601_01[] = "LED1";
static const char acName1A00[] = "Buttons";
static const char acName1A00_00[] = "Max SubIndex";
static const char acName1A00_01[] = "Button1";
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
static const char acName1C13[] = "Sync Manager 3 PDO Assignment";
static const char acName1C13_00[] = "Max SubIndex";
static const char acName1C13_01[] = "PDO Mapping";
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
static const char acName6005[] = "Buttons";
static const char acName6005_00[] = "Max SubIndex";
static const char acName6005_01[] = "Button1";
static const char acName7005[] = "LEDgroup0";
static const char acName7005_00[] = "Max SubIndex";
static const char acName7005_01[] = "LED0";
static const char acName7006[] = "LEDgroup1";
static const char acName7006_00[] = "Max SubIndex";
static const char acName7006_01[] = "LED1";
static const char acName8000[] = "Parameters";
static const char acName8000_00[] = "Max SubIndex";
static const char acName8000_01[] = "Multiplier";
static const char acName8001[] = "variableRW";
static const char acName8001_0[] = "variableRW";

const _objd SDO1000[] =
{
  {0x0, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1000_0, 0x00001389, NULL},
};
const _objd SDO1008[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 88, ATYPE_RO, acName1008_0, 0, "xmc43_slave"},
};
const _objd SDO1009[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 24, ATYPE_RO, acName1009_0, 0, "1.0"},
};
const _objd SDO100A[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 24, ATYPE_RO, acName100A_0, 0, "1.0"},
};
const _objd SDO1018[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1018_00, 4, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_01, 0x1337, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_02, 4300, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_03, 0, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_04, 0x00000000, NULL},
};
const _objd SDO10F1[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName10F1_00, 2, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName10F1_01, 0, &Mb.ErrorSettings.Dummy_x01},
  {0x02, DTYPE_UNSIGNED16, 16, ATYPE_RW, acName10F1_02, 2, &Mb.ErrorSettings.SyncErrorCounterLimit},
};
const _objd SDO1600[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_01, 0x70050108, NULL},
};
const _objd SDO1601[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1601_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1601_01, 0x70060108, NULL},
};
const _objd SDO1A00[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A00_01, 0x60050108, NULL},
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
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C12_00, 2, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C12_01, 0x1600, NULL},
  {0x02, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C12_02, 0x1601, NULL},
};
const _objd SDO1C13[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C13_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C13_01, 0x1A00, NULL},
};
const _objd SDO1C32[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C32_00, 32, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_01, 1, &Mb.SyncMgrParam.SyncType},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_02, 0, &Mb.SyncMgrParam.CycleTime},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_03, 0, &Mb.SyncMgrParam.ShiftTime},
  {0x04, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_04, 0x6, &Mb.SyncMgrParam.SyncTypeSupport},
  {0x05, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_05, 125000, &Mb.SyncMgrParam.MinCycleTime},
  {0x06, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_06, 0, &Mb.SyncMgrParam.CalcCopyTime},
  {0x07, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_07, 0, &Mb.SyncMgrParam.MinDelayTime},
  {0x08, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_08, 0, &Mb.SyncMgrParam.GetCycleTime},
  {0x09, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_09, 0, &Mb.SyncMgrParam.DelayTime},
  {0x0A, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_0A, 0, &Mb.SyncMgrParam.Sync0CycleTime},
  {0x0B, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_0B, 0, &Mb.SyncMgrParam.SMEventMissedCnt},
  {0x0C, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_0C, 0, &Mb.SyncMgrParam.CycleTimeTooSmallCnt},
  {0x0D, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_0D, 0, &Mb.SyncMgrParam.ShiftTimeTooSmallCnt},
  {0x0E, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C32_0E, 0, &Mb.SyncMgrParam.RxPDOToggleFailed},
  {0x0F, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_0F, 0, &Mb.SyncMgrParam.MinCycleDist},
  {0x10, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_10, 0, &Mb.SyncMgrParam.MaxCycleDist},
  {0x11, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_11, 0, &Mb.SyncMgrParam.MinSMSYNCDist},
  {0x12, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1C32_12, 0, &Mb.SyncMgrParam.MaxSMSYNCDist},
  {0x14, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C32_14, 0, &Mb.SyncMgrParam.Dummy_x14},
  {0x20, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C32_20, 0, &Mb.SyncMgrParam.SyncError},
};
const _objd SDO6005[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName6005_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName6005_01, 0, &Rb.Buttons.Button1},
};
const _objd SDO7005[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName7005_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName7005_01, 0, &Wb.LEDgroup0.LED0},
};
const _objd SDO7006[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName7006_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName7006_01, 0, &Wb.LEDgroup1.LED1},
};
const _objd SDO8000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName8000_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, acName8000_01, 0, &Cb.Parameters.Multiplier},
};
const _objd SDO8001[] =
{
  {0x0, DTYPE_UNSIGNED32, 32, ATYPE_RW, acName8001_0, 0, &Cb.variableRW},
};

const _objectlist SDOobjects[] =
{
  {0x1000, OTYPE_VAR, 0, 0, acName1000, SDO1000},
  {0x1008, OTYPE_VAR, 0, 0, acName1008, SDO1008},
  {0x1009, OTYPE_VAR, 0, 0, acName1009, SDO1009},
  {0x100A, OTYPE_VAR, 0, 0, acName100A, SDO100A},
  {0x1018, OTYPE_RECORD, 4, 0, acName1018, SDO1018},
  {0x10F1, OTYPE_RECORD, 2, 0, acName10F1, SDO10F1},
  {0x1600, OTYPE_RECORD, 1, 0, acName1600, SDO1600},
  {0x1601, OTYPE_RECORD, 1, 0, acName1601, SDO1601},
  {0x1A00, OTYPE_RECORD, 1, 0, acName1A00, SDO1A00},
  {0x1C00, OTYPE_ARRAY, 4, 0, acName1C00, SDO1C00},
  {0x1C12, OTYPE_ARRAY, 2, 0, acName1C12, SDO1C12},
  {0x1C13, OTYPE_ARRAY, 1, 0, acName1C13, SDO1C13},
  {0x1C32, OTYPE_RECORD, 32, 0, acName1C32, SDO1C32},
  {0x6005, OTYPE_RECORD, 1, 0, acName6005, SDO6005},
  {0x7005, OTYPE_RECORD, 1, 0, acName7005, SDO7005},
  {0x7006, OTYPE_RECORD, 1, 0, acName7006, SDO7006},
  {0x8000, OTYPE_RECORD, 1, 0, acName8000, SDO8000},
  {0x8001, OTYPE_VAR, 0, 0, acName8001, SDO8001},
  {0xffff, 0xff, 0xff, 0xff, NULL, NULL}
};
#endif