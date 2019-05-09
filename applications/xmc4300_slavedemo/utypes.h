/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
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

CC_PACKED_BEGIN
typedef struct
{
   uint8_t state;
   uint8_t button;
   uint32_t encoder;
}CC_PACKED _Rbuffer;
CC_PACKED_END

CC_PACKED_BEGIN
typedef struct
{
   uint8_t LED;
}CC_PACKED _Wbuffer;
CC_PACKED_END

CC_PACKED_BEGIN
typedef struct
{
   uint32_t reset_counter;
}CC_PACKED _Cbuffer;
CC_PACKED_END

#endif
