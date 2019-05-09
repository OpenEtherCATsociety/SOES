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
static const char acName1008[] = "Device Name";
static const char acName1009[] = "Hardware Version";
static const char acName100A[] = "Software Version";
static const char acName1018[] = "Identity Object";
static const char acName1018_00[] = "Max SubIndex";
static const char acName1018_01[] = "Vendor ID";
static const char acName1018_02[] = "Product Code";
static const char acName1018_03[] = "Revision Number";
static const char acName1018_04[] = "Serial Number";
static const char acName1600[] = "LED";
static const char acName1600_00[] = "Max SubIndex";
static const char acName1600_01[] = "LED";
static const char acName1A00[] = "BUTTON";
static const char acName1A00_00[] = "Max SubIndex";
static const char acName1A00_01[] = "BUTTON";
static const char acName1C00[] = "Sync Manager Communication Type";
static const char acName1C00_00[] = "Max SubIndex";
static const char acName1C00_01[] = "Communications Type SM0";
static const char acName1C00_02[] = "Communications Type SM1";
static const char acName1C00_03[] = "Communications Type SM2";
static const char acName1C00_04[] = "Communications Type SM3";
static const char acName1C12[] = "Sync Manager 2 PDO Assignment";
static const char acName1C12_00[] = "Max SubIndex";
static const char acName1C12_01[] = "PDO Mapping";
static const char acName1C13[] = "Sync Manager 3 PDO Assignment";
static const char acName1C13_00[] = "Max SubIndex";
static const char acName1C13_01[] = "PDO Mapping";
static const char acName6000[] = "BUTTON";
static const char acName7000[] = "LED";

const _objd SDO1000[] =
{
  {0x0, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1000, 0x00001389, NULL},
};
const _objd SDO1008[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 48, ATYPE_RO, acName1008, 0, "k2gice"},
};
const _objd SDO1009[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 24, ATYPE_RO, acName1009, 0, HW_REV},
};
const _objd SDO100A[] =
{
  {0x0, DTYPE_VISIBLE_STRING, 24, ATYPE_RO, acName100A, 0, SW_REV},
};
const _objd SDO1018[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1018_00, 4, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_01, 0x1337, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_02, 0x4291ce, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_03, 0, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1018_04, 0x00000000, NULL},
};
const _objd SDO1600[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1600_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1600_01, 0x70000020, NULL},
};
const _objd SDO1A00[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1A00_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acName1A00_01, 0x60000020, NULL},
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
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C12_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C12_01, 0x1600, NULL},
};
const _objd SDO1C13[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acName1C13_00, 1, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acName1C13_01, 0x1A00, NULL},
};
const _objd SDO6000[] =
{
  {0x0, DTYPE_UNSIGNED32, 32, ATYPE_RO | ATYPE_TXPDO, acName6000, 0, &Obj.BUTTON},
};
const _objd SDO7000[] =
{
  {0x0, DTYPE_UNSIGNED32, 32, ATYPE_RO | ATYPE_RXPDO, acName7000, 0, &Obj.LED},
};

const _objectlist SDOobjects[] =
{
  {0x1000, OTYPE_VAR, 0, 0, acName1000, SDO1000},
  {0x1008, OTYPE_VAR, 0, 0, acName1008, SDO1008},
  {0x1009, OTYPE_VAR, 0, 0, acName1009, SDO1009},
  {0x100A, OTYPE_VAR, 0, 0, acName100A, SDO100A},
  {0x1018, OTYPE_RECORD, 4, 0, acName1018, SDO1018},
  {0x1600, OTYPE_RECORD, 1, 0, acName1600, SDO1600},
  {0x1A00, OTYPE_RECORD, 1, 0, acName1A00, SDO1A00},
  {0x1C00, OTYPE_ARRAY, 4, 0, acName1C00, SDO1C00},
  {0x1C12, OTYPE_ARRAY, 1, 0, acName1C12, SDO1C12},
  {0x1C13, OTYPE_ARRAY, 1, 0, acName1C13, SDO1C13},
  {0x6000, OTYPE_VAR, 0, 0, acName6000, SDO6000},
  {0x7000, OTYPE_VAR, 0, 0, acName7000, SDO7000},
  {0xffff, 0xff, 0xff, 0xff, NULL, NULL}
};
