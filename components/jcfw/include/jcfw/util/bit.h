#ifndef __JCFW_UTIL_BIT_H__
#define __JCFW_UTIL_BIT_H__

#include <endian.h>

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
