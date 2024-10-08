#ifndef __JCFW_UTIL_MATH_H__
#define __JCFW_UTIL_MATH_H__

// TODO(Caleb): Make a better place for this
#define JCFW_ARRAYSIZE(_arr)        (sizeof(_arr) / sizeof(_arr[0]))

#define JCFW_MAX(_a, _b)            (((_a) >= (_b)) ? (_a) : (_b))
#define JCFW_MIN(_a, _b)            (((_a) <= (_b)) ? (_a) : (_b))
#define JCFW_ABS(_v)                (((_v) < 0) ? (-_v) : (_v))
#define JCFW_CLAMP(_x, _low, _high) (JCFW_MIN(_high, JCFW_MAX(_low, _x)))

#define JCFW_MAX_VAL(_bits)         (uint32_t)((1 << _bits) - 1)
#define JCFW_U8_MAX                 JCFW_MAX_VAL(8)
#define JCFW_U16_MAX                JCFW_MAX_VAL(16)
#define JCFW_U32_MAX                JCFW_MAX_VAL(32)
#define JCFW_U64_MAX                JCFW_MAX_VAL(64)

#endif // __JCFW_UTIL_MATH_H__
