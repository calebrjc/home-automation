#ifndef __JCFW_PLATFORM_DETAIL_COMPILER_H__
#define __JCFW_PLATFORM_DETAIL_COMPILER_H__

// Likely/Unlikely traits
#ifdef __GNUC__
#define JCFW_LIKELY(x)   __builtin_expect(!!(x), 1)
#define JCFW_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define JCFW_LIKELY(x)   (x)
#define JCFW_UNLIKELY(x) (x)
#endif

#endif // __JCFW_PLATFORM_COMPILER_H__
