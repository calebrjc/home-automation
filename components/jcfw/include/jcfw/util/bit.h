#ifndef __JCFW_UTIL_BIT_H__
#define __JCFW_UTIL_BIT_H__

#define JCFW_LITTLE_ENDIAN 1234
#define JCFW_BIG_ENDIAN    4321

#if __has_include("endian.h")
#include <endian.h>
#else
#ifndef JCFW_BYTE_ORDER
#error "No endian.h found. Please #define JCFW_BYTE_ORDER as JCFW_LITTLE_ENDIAN or JFCW_BIG_ENDIAN"
#else // _BYTE_ORDER
#include <stdint.h>

static inline uint16_t __bswap16(uint16_t x)
{
    return ((uint16_t)((x >> 8) | ((x << 8) & 0xff00)));
}

static inline uint32_t __bswap32(uint32_t x)
{
    return ((uint32_t)((x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000)
                       | ((x << 24) & 0xff000000)));
}

#if JCFW_BYTE_ORDER == JCFW_LITTLE_ENDIAN
#define __htonl(_x) __bswap32(_x)
#define __htons(_x) __bswap16(_x)
#define __ntohl(_x) __bswap32(_x)
#define __ntohs(_x) __bswap16(_x)
#else
#define __htonl(_x) ((uint32_t)(_x))
#define __htons(_x) ((uint16_t)(_x))
#define __ntohl(_x) ((uint32_t)(_x))
#define __ntohs(_x) ((uint16_t)(_x))
#endif // _BYTE_ORDER == _LITTLE_ENDIAN
#endif // _BYTE_ORDER
#endif // __has_include

#define JCFW_BIT(_n)           (1UL << (_n))
#define JCFW_BITSET(_v, _m)    (_v) |= (_m)
#define JCFW_BITCLEAR(_v, _m)  (_v) &= ~(_m)
#define JCFW_BITTOGGLE(_v, _m) (_v) ^= (_m)

#define JCFW_BSWAP16(_v)       __bswap16(_v)
#define JCFW_BSWAP32(_v)       __bswap32(_v)

#define JCFW_HTONS(_v)         __htons(_v)
#define JCFW_NTOHS(_v)         __ntohs(_v)
#define JCFW_HTONL(_v)         __htonl(_v)
#define JCFW_NTOHL(_v)         __ntohl(_v)

#endif //  __JCFW_UTIL_BIT_H__
