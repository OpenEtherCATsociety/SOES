/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#ifndef CC_H
#define CC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <sys/param.h>
#ifdef __linux__
   #include <endian.h>
#else
   #include <machine/endian.h>   
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define CC_PACKED_BEGIN
#define CC_PACKED_END
#define CC_PACKED       __attribute__((packed))
#define CC_ALIGNED(n)   __attribute__((aligned (n)))

#ifdef __rtk__
#include <kern/assert.h>
#define CC_ASSERT(exp) ASSERT (exp)
#else
#define CC_ASSERT(exp) assert (exp)
#endif
#define CC_STATIC_ASSERT(exp) _Static_assert (exp, "")

#define CC_DEPRECATED   __attribute__((deprecated))

#define CC_SWAP32(x) __builtin_bswap32 (x)
#define CC_SWAP16(x) __builtin_bswap16 (x)

#define CC_ATOMIC_SET(var,val)   __atomic_store_n(&var,val,__ATOMIC_SEQ_CST)
#define CC_ATOMIC_GET(var)       __atomic_load_n(&var,__ATOMIC_SEQ_CST)
#define CC_ATOMIC_ADD(var,val)   __atomic_add_fetch(&var,val,__ATOMIC_SEQ_CST)
#define CC_ATOMIC_SUB(var,val)   __atomic_sub_fetch(&var,val,__ATOMIC_SEQ_CST)
#define CC_ATOMIC_AND(var,val)   __atomic_and_fetch(&var,val,__ATOMIC_SEQ_CST)
#define CC_ATOMIC_OR(var,val)    __atomic_or_fetch(&var,val,__ATOMIC_SEQ_CST)

#if BYTE_ORDER == BIG_ENDIAN
#define htoes(x) CC_SWAP16 ((uint16_t)(x))
#define htoel(x) CC_SWAP32 ((uint32_t)(x))
#else
#define htoes(x) ((uint16_t)(x))
#define htoel(x) ((uint32_t)(x))
#endif

#define etohs(x) htoes (x)
#define etohl(x) htoel (x)

#if BYTE_ORDER == LITTLE_ENDIAN
#define EC_LITTLE_ENDIAN
#else
#define EC_BIG_ENDIAN
#endif

#ifdef ESC_DEBUG
#ifdef __rtk__
#include <kern/rprint.h>
#define DPRINT(...) rprintp ("soes: "__VA_ARGS__)
#else
#include <stdio.h>
#define DPRINT(...) printf ("soes: "__VA_ARGS__)
#endif
#else
#define DPRINT(...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* CC_H */
