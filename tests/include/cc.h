/*
 * Minimal host cc.h for the standalone SOES tests.
 * Licensed under the GNU General Public License version 2 with exceptions.
 */

#ifndef TEST_CC_H
#define TEST_CC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CC_PACKED_BEGIN
#define CC_PACKED_END
#define CC_PACKED __attribute__((packed))
#define CC_ALIGNED(n) __attribute__((aligned(n)))
#define CC_STATIC_ASSERT(exp, msg) _Static_assert(exp, msg)
#define CC_ASSERT(exp) do { if (!(exp)) __builtin_trap(); } while (0)
#define CC_DEPRECATED __attribute__((deprecated))
#define CC_SWAP32(x) __builtin_bswap32(x)
#define CC_SWAP16(x) __builtin_bswap16(x)

#define EC_LITTLE_ENDIAN
#define htoes(x) ((uint16_t)(x))
#define htoel(x) ((uint32_t)(x))
#define etohs(x) htoes(x)
#define etohl(x) htoel(x)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define DPRINT(...)

#endif
