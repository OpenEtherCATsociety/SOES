/*
 * SOES Simple Open EtherCAT Slave
 *
 * File    : utype.h
 * Version : 0.9.2
 * Date    : 22-02-2010
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
 * Header file for Application types. 
 *
 * Part of the application, host declarations of application types mapped against
 * ServiceData Objects and ProcessData Objects .
 */
 
#include <stdint.h>

#define uint8   uint8_t
#define int8    int8_t
#define uint16  uint16_t
#define int16   int16_t
#define uint32  uint32_t
#define int32   int32_t

typedef union
{
   uint16 w;
   uint8 b[2];
} uint16union;

#define APPSTATE_IDLE      0x00
#define APPSTATE_INPUT     0x01
#define APPSTATE_OUTPUT    0x02

typedef struct
{
   uint8 state;
   uint8 button;
   uint32 encoder;
} _Rbuffer;

typedef struct
{
   uint8 LED;
} _Wbuffer;

typedef struct
{
   uint32 reset_counter;
} _Cbuffer;

typedef struct
{
   uint8 state;                 // internal, don't change!!!!
} _App;

extern _Rbuffer Rb;
extern _Wbuffer Wb;
extern _Cbuffer Cb;
extern _App App;
extern uint32 encoder_scale;
extern uint32 encoder_scale_mirror;
#define EC_LITTLE_ENDIAN

#if !defined(EC_BIG_ENDIAN) && defined(EC_LITTLE_ENDIAN)

#define htoes(A) (A)
#define htoel(A) (A)
#define htoell(A) (A)
#define etohs(A) (A)
#define etohl(A) (A)
#define etohll(A) (A)

#elif !defined(EC_LITTLE_ENDIAN) && defined(EC_BIG_ENDIAN)

#define htoes(A) ((((uint16)(A) & 0xff00) >> 8) | \
                    (((uint16)(A) & 0x00ff) << 8))
#define htoel(A) ((((uint32)(A) & 0xff000000) >> 24) | \
                    (((uint32)(A) & 0x00ff0000) >> 8)  | \
                    (((uint32)(A) & 0x0000ff00) << 8)  | \
                    (((uint32)(A) & 0x000000ff) << 24))
#define htoell(A) ((((uint64)(A) & (uint64)0xff00000000000000ULL) >> 56) | \
                     (((uint64)(A) & (uint64)0x00ff000000000000ULL) >> 40) | \
                     (((uint64)(A) & (uint64)0x0000ff0000000000ULL) >> 24) | \
                     (((uint64)(A) & (uint64)0x000000ff00000000ULL) >> 8)  | \
                     (((uint64)(A) & (uint64)0x00000000ff000000ULL) << 8)  | \
                     (((uint64)(A) & (uint64)0x0000000000ff0000ULL) << 24) | \
                     (((uint64)(A) & (uint64)0x000000000000ff00ULL) << 40) | \
                     (((uint64)(A) & (uint64)0x00000000000000ffULL) << 56))

#define etohs  htoes
#define etohl  htoel
#define etohll htoell

#else

#error "Must define one of EC_BIG_ENDIAN or EC_LITTLE_ENDIAN"

#endif
