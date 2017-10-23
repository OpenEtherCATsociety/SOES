#ifndef CC_H
#define CC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include <stdint.h>
#ifdef __linux__
   #include <endian.h>
#else
   #include <machine/endian.h>   
#endif

#define CC_PACKED_BEGIN
#define CC_PACKED_END
#define CC_PACKED       __attribute__((packed))

#define CC_ASSERT(exp) assert (exp)
#define CC_STATIC_ASSERT(exp) _Static_assert (exp, "")

#define CC_SWAP32(x) __builtin_bswap32 (x)
#define CC_SWAP16(x) ((uint16_t)(x) >> 8 | ((uint16_t)(x) & 0xFF) << 8)

#define CC_ATOMIC_SET(var,val)   __atomic_store_n(&var,val,__ATOMIC_SEQ_CST)
#define CC_ATOMIC_GET(var)       __atomic_load_n(&var,__ATOMIC_SEQ_CST)
#define CC_ATOMIC_ADD(var,val)   __atomic_add_fetch(&var,val,__ATOMIC_SEQ_CST)
#define CC_ATOMIC_SUB(var,val)   __atomic_sub_fetch(&var,val,__ATOMIC_SEQ_CST)
#define CC_ATOMIC_AND(var,val)   __atomic_and_fetch(&var,val,__ATOMIC_SEQ_CST)
#define CC_ATOMIC_OR(var,val)    __atomic_or_fetch(&var,val,__ATOMIC_SEQ_CST)

#if BYTE_ORDER == BIG_ENDIAN
#define htoes(x) CC_SWAP16 (x)
#define htoel(x) CC_SWAP32 (x)
#else
#define htoes(x) (x)
#define htoel(x) (x)
#endif

#define etohs(x) htoes (x)
#define etohl(x) htoel (x)

#if BYTE_ORDER == LITTLE_ENDIAN
#define EC_LITTLE_ENDIAN
#else
#define EC_BIG_ENDIAN
#endif

#ifdef ESC_DEBUG
#define DPRINT(...) rprintp ("esc_foe: "__VA_ARGS__) /* TODO */
#else
#define DPRINT(...)
#endif  /* DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* CC_H */
