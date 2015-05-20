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
 
#ifndef __objectlist__
#define __objectlist__

typedef FLASHSTORE struct PACKED
{
   uint16 subindex;
   uint16 datatype;
   uint16 bitlength;
   uint16 access;
   char FLASHSTORE *name;
   uint32 value;
   void *data;
} _objd;

typedef FLASHSTORE struct PACKED
{
   uint16 index;
   uint16 objtype;
   uint8 maxsub;
   uint8 pad1;
   char FLASHSTORE *name;
   _objd FLASHSTORE *objdesc;
} _objectlist;

#define OBJH_READ               0
#define OBJH_WRITE              1

#define _ac FLASHSTORE char
#define nil 0

#define OTYPE_DOMAIN            0x0002
#define OTYPE_DEFTYPE           0x0005
#define OTYPE_DEFSTRUCT         0x0006
#define OTYPE_VAR               0x0007
#define OTYPE_ARRAY             0x0008
#define OTYPE_RECORD            0x0009

#define DTYPE_BOOLEAN           0x0001
#define DTYPE_INTEGER8          0x0002
#define DTYPE_INTEGER16         0x0003
#define DTYPE_INTEGER32         0x0004
#define DTYPE_UNSIGNED8         0x0005
#define DTYPE_UNSIGNED16        0x0006
#define DTYPE_UNSIGNED32        0x0007
#define DTYPE_REAL32            0x0008
#define DTYPE_VISIBLE_STRING    0x0009
#define DTYPE_OCTET_STRING      0x000A
#define DTYPE_UNICODE_STRING    0x000B
#define DTYPE_INTEGER24         0x0010
#define DTYPE_UNSIGNED24        0x0016
#define DTYPE_INTEGER64         0x0015
#define DTYPE_UNSIGNED64        0x001B
#define DTYPE_REAL64            0x0011
#define DTYPE_PDO_MAPPING       0x0021
#define DTYPE_IDENTITY          0x0023
#define DTYPE_BIT1              0x0030
#define DTYPE_BIT2              0x0031
#define DTYPE_BIT3              0x0032
#define DTYPE_BIT4              0x0033
#define DTYPE_BIT5              0x0034
#define DTYPE_BIT6              0x0035
#define DTYPE_BIT7              0x0036
#define DTYPE_BIT8              0x0037

#define ATYPE_R                 0x07
#define ATYPE_RW                0x3F
#define ATYPE_RWpre             0x0F
#define ATYPE_RXPDO             0x40
#define ATYPE_TXPDO             0x80

_ac acName1000[] = "Device Type";
_ac acName1008[] = "Manufacturer Device Name";
_ac acName1009[] = "Manufacturer Hardware Version";
_ac acName100A[] = "Manufacturer Software Version";
_ac acName1018[] = "Identity Object";
_ac acName1018_01[] = "Vendor ID";
_ac acName1018_02[] = "Product Code";
_ac acName1018_03[] = "Revision Number";
_ac acName1018_04[] = "Serial Number";
_ac acNameMO[] = "Mapped object";
_ac acName1600[] = "Receive PDO mapping";
_ac acName1A00[] = "Transmit PDO mapping";
_ac acName1C00[] = "Sync Manager Communication type";
_ac acName1C00_01[] = "Communications type SM0";
_ac acName1C00_02[] = "Communications type SM1";
_ac acName1C00_03[] = "Communications type SM2";
_ac acName1C00_04[] = "Communications type SM3";
_ac acName1C10[] = "Sync Manager 0 PDO Assignment";
_ac acName1C11[] = "Sync Manager 1 PDO Assignment";
_ac acName1C12[] = "Sync Manager 2 PDO Assignment";
_ac acName1C13[] = "Sync Manager 3 PDO Assignment";
_ac acNameNOE[] = "Number of entries";
_ac acName6000[] = "Digital Inputs";
_ac acName6000_01[] = "Button";
_ac acName6000_02[] = "Encoder";
_ac acName7000[] = "Digital outputs";
_ac acName7000_01[] = "LED";
_ac acName7100[] = "Parameters";
_ac acName7100_01[] = "Encoder scale";
_ac acName7100_02[] = "Encoder scale mirror";
_ac acName8001[] = "Slave commands";
_ac acName8001_01[] = "Reset counter";
//_ac acName[]="";

char ac1008_00[] = "SOES test application";
char ac1009_00[] = "0.0.2";
char ac100A_00[] = "0.9.3";

FLASHSTORE _objd SDO1000[] =
{ {0x00, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName1000[0], 0x00000000, nil} };
FLASHSTORE _objd SDO1008[] =
{ {0x00, DTYPE_VISIBLE_STRING, sizeof (ac1008_00) << 3, ATYPE_R, &acName1008[0], 0, &ac1008_00[0]}
};
FLASHSTORE _objd SDO1009[] =
{ {0x00, DTYPE_VISIBLE_STRING, sizeof (ac1009_00) << 3, ATYPE_R, &acName1009[0], 0, &ac1009_00[0]}
};
FLASHSTORE _objd SDO100A[] =
{ {0x00, DTYPE_VISIBLE_STRING, sizeof (ac100A_00) << 3, ATYPE_R, &acName100A[0], 0, &ac100A_00[0]}
};
FLASHSTORE _objd SDO1018[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x04, nil},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName1018_01[0], 0x00001337, nil},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName1018_02[0], 0x12783456, nil},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName1018_03[0], 0x00000001, nil},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName1018_04[0], 0x00000000, nil}
};
FLASHSTORE _objd SDO1600[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x01, nil},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_R, &acNameMO[0], 0x70000108, nil}
};
FLASHSTORE _objd SDO1A00[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x02, nil},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_R, &acNameMO[0], 0x60000108, nil},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_R, &acNameMO[0], 0x60000220, nil}

};
FLASHSTORE _objd SDO1C00[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x04, nil},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C00_01[0], 0x01, nil},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C00_02[0], 0x02, nil},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C00_03[0], 0x03, nil},
  {0x04, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C00_04[0], 0x04, nil}
};
FLASHSTORE _objd SDO1C10[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C10[0], 0x00, nil}
};
FLASHSTORE _objd SDO1C11[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName1C11[0], 0x00, nil}
};
FLASHSTORE _objd SDO1C12[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x01, nil},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_R, &acNameMO[0], 0x1600, nil}
};
FLASHSTORE _objd SDO1C13[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x01, nil},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_R, &acNameMO[0], 0x1A00, nil}
};
FLASHSTORE _objd SDO6000[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x02, nil},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_R, &acName6000_01[0], 0, &(Rb.button)},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName6000_02[0], 0, &(Rb.encoder)}
};
FLASHSTORE _objd SDO7000[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x01, nil},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RW, &acName7000_01[0], 0, &(Wb.LED)}
};
FLASHSTORE _objd SDO7100[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x02, nil},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, &acName7100_01[0], 0, &(encoder_scale)},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_R, &acName7100_02[0], 0, &(encoder_scale_mirror)}
};

FLASHSTORE _objd SDO8001[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_R, &acNameNOE[0], 0x01, nil},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, &acName8001_01[0], 0, &(Cb.reset_counter)},
};

FLASHSTORE _objectlist SDOobjects[] =
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
  {0xffff, 0xff, 0xff, 0xff, nil, nil}
};

#endif
