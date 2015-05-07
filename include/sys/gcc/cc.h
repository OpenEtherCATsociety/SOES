#ifndef CC_H
#define CC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include <stdint.h>

typedef uint8_t  uint8;
typedef int8_t   int8;
typedef uint16_t uint16;
typedef int16_t  int16;
typedef uint32_t uint32;
typedef int32_t  int32;

#define CC_PACKED_BEGIN
#define CC_PACKED_END
#define CC_PACKED       __attribute__((packed))

#define CC_ASSERT(exp) assert (exp)
#define CC_STATIC_ASSERT(exp) _Static_assert (exp, "")

#define CC_SWAP32(x) __builtin_bswap32 (x)
#define CC_SWAP16(x) ((uint16_t)(x) >> 8 | ((uint16_t)(x) & 0xFF) << 8)

#if BYTE_ORDER == LITTLE_ENDIAN
#define htoes(x) CC_SWAP16 (x)
#define htoel(x) CC_SWAP32 (x)
#define etohs(x) htoes (x)
#define etohl(x) htoel (x)
#else
#define htoes(x) (x)
#define htoel(x) (x)
#define etohs(x) (x)
#define etohl(x) (x)
#endif

#ifdef __cplusplus
}
#endif

#endif /* CC_H */
