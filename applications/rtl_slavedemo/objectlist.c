/*
 * SOES Simple Open EtherCAT Slave
 *
 * Copyright (C) 2007-2013 Arthur Ketels
 *
 * SOES is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * SOES is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * As a special exception, if other files instantiate templates or use macros
 * or inline functions from this file, or you compile this file and link it
 * with other works to produce a work based on this file, this file does not
 * by itself cause the resulting work to be covered by the GNU General Public
 * License. However the source code for this file must still be made available
 * in accordance with section (3) of the GNU General Public License.
 *
 * This exception does not invalidate any other reasons why a work based on
 * this file might be covered by the GNU General Public License.
 *
 * The EtherCAT Technology, the trade name and logo "EtherCAT" are the intellectual
 * property of, and protected by Beckhoff Automation GmbH.
 */

 /** \file
 * \brief
 * CoE Object Dictionary.
 *
 * Part of application, describe the slave and its process data.
 */

#include "esc_coe.h"
#include "utypes.h"
#include <stddef.h>

extern _Rbuffer Rb;
extern _Wbuffer Wb;
extern _Cbuffer Cb;
extern uint32_t encoder_scale;
extern uint32_t encoder_scale_mirror;

static const char acName1000[] = "Device Type";
static const char acName1008[] = "Manufacturer Device Name";
static const char acName1009[] = "Manufacturer Hardware Version";
static const char acName100A[] = "Manufacturer Software Version";
static const char acName1018[] = "Identity Object";
static const char acName1018_01[] = "Vendor ID";
static const char acName1018_02[] = "Product Code";
static const char acName1018_03[] = "Revision Number";
static const char acName1018_04[] = "Serial Number";
static const char acNameMO[] = "Mapped object";
static const char acName1600[] = "Receive PDO mapping";
static const char acName1A00[] = "Transmit PDO mapping";
static const char acName1C00[] = "Sync Manager Communication type";
static const char acName1C00_01[] = "Communications type SM0";
static const char acName1C00_02[] = "Communications type SM1";
static const char acName1C00_03[] = "Communications type SM2";
static const char acName1C00_04[] = "Communications type SM3";
static const char acName1C10[] = "Sync Manager 0 PDO Assignment";
static const char acName1C11[] = "Sync Manager 1 PDO Assignment";
static const char acName1C12[] = "Sync Manager 2 PDO Assignment";
static const char acName1C13[] = "Sync Manager 3 PDO Assignment";
static const char acNameNOE[] = "Number of entries";
static const char acName6000[] = "Digital Inputs";
static const char acName6000_01[] = "Button";
static const char acName6000_02[] = "Encoder";
static const char acName7000[] = "Digital outputs";
static const char acName7000_01[] = "LED";
static const char acName7100[] = "Parameters";
static const char acName7100_01[] = "Encoder scale";
static const char acName7100_02[] = "Encoder scale mirror";
static const char acName8001[] = "Slave commands";
static const char acName8001_01[] = "Reset counter";

static char ac1008_00[] = "SOES test application";
static char ac1009_00[] = "0.0.2";
static char ac100A_00[] = "0.9.3";

const _objd SDO1000[] =
{ {0x00, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName1000[0], 0x00000000, NULL} };
const _objd SDO1008[] =
{ {0x00, DTYPE_VISIBLE_STRING, sizeof (ac1008_00) << 3, ATYPE_R, &acName1008[0], 0, &ac1008_00[0]}
};
const _objd SDO1009[] =
{ {0x00, DTYPE_VISIBLE_STRING, sizeof (ac1009_00) << 3, ATYPE_R, &acName1009[0], 0, &ac1009_00[0]}
};
const _objd SDO100A[] =
{ {0x00, DTYPE_VISIBLE_STRING, sizeof (ac100A_00) << 3, ATYPE_R, &acName100A[0], 0, &ac100A_00[0]}
};
const _objd SDO1018[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x04, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName1018_01[0], 0x00001337, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName1018_02[0], 0x12783456, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName1018_03[0], 0x00000001, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName1018_04[0], 0x00000000, NULL}
};
const _objd SDO1600[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x01, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_R, &acNameMO[0], 0x70000108, NULL}
};
const _objd SDO1A00[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x02, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_R, &acNameMO[0], 0x60000108, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_R, &acNameMO[0], 0x60000220, NULL}

};
const _objd SDO1C00[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x04, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C00_01[0], 0x01, NULL},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C00_02[0], 0x02, NULL},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C00_03[0], 0x03, NULL},
  {0x04, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C00_04[0], 0x04, NULL}
};
const _objd SDO1C10[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C10[0], 0x00, NULL}
};
const _objd SDO1C11[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C11[0], 0x00, NULL}
};
const _objd SDO1C12[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x01, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_R, &acNameMO[0], 0x1600, NULL}
};
const _objd SDO1C13[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x01, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_R, &acNameMO[0], 0x1A00, NULL}
};
const _objd SDO6000[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x02, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName6000_01[0], 0, &(Rb.button)},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName6000_02[0], 0, &(Rb.encoder)}
};
const _objd SDO7000[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x01, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RW, &acName7000_01[0], 0, &(Wb.LED)}
};
const _objd SDO7100[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x02, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, &acName7100_01[0], 0, &(encoder_scale)},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName7100_02[0], 0, &(encoder_scale_mirror)}
};

const _objd SDO8001[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x01, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, &acName8001_01[0], 0, &(Cb.reset_counter)},
};

const _objectlist SDOobjects[] =
{ {0x1000, OTYPE_VAR, 0, 0, &acName1000[0], &SDO1000[0]},
  {0x1008, OTYPE_VAR, 0, 0, &acName1008[0], &SDO1008[0]},
  {0x1009, OTYPE_VAR, 0, 0, &acName1009[0], &SDO1009[0]},
  {0x100A, OTYPE_VAR, 0, 0, &acName100A[0], &SDO100A[0]},
  {0x1018, OTYPE_RECORD, 4, 0, &acName1018[0], &SDO1018[0]},
  {0x1600, OTYPE_RECORD, 0x01, 0, &acName1600[0], &SDO1600[0]},
  {0x1A00, OTYPE_RECORD, 0x02, 0, &acName1A00[0], &SDO1A00[0]},
  {0x1C00, OTYPE_ARRAY, 4, 0, &acName1C00[0], &SDO1C00[0]},
  {0x1C10, OTYPE_ARRAY, 0, 0, &acName1C10[0], &SDO1C10[0]},
  {0x1C11, OTYPE_ARRAY, 0, 0, &acName1C11[0], &SDO1C11[0]},
  {0x1C12, OTYPE_ARRAY, 1, 0, &acName1C12[0], &SDO1C12[0]},
  {0x1C13, OTYPE_ARRAY, 1, 0, &acName1C13[0], &SDO1C13[0]},
  {0x6000, OTYPE_ARRAY, 0x02, 0, &acName6000[0], &SDO6000[0]},
  {0x7000, OTYPE_ARRAY, 0x01, 0, &acName7000[0], &SDO7000[0]},
  {0x7100, OTYPE_ARRAY, 0x02, 0, &acName7100[0], &SDO7100[0]},
  {0x8001, OTYPE_ARRAY, 0x01, 0, &acName8001[0], &SDO8001[0]},
  {0xffff, 0xff, 0xff, 0xff, NULL, NULL}
};
