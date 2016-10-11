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
 * Header file for Application types.
 *
 * Part of the application, host declarations of application types mapped against
 * ServiceData Objects and ProcessData Objects.
 */

#ifndef __utypes_h__
#define __utypes_h__

#include <cc.h>

typedef struct
{
   uint8_t state;
   uint8_t button;
   uint32_t encoder;
} _Rbuffer;

typedef struct
{
   uint8_t LED;
} _Wbuffer;

typedef struct
{
   uint32_t reset_counter;
} _Cbuffer;

#endif
