#ifndef __JCFW_UTIL_ASSERT_H__
#define __JCFW_UTIL_ASSERT_H__

#include "jcfw/platform/compiler.h"
#include "jcfw/platform/platform.h"
#include "jcfw/trace.h"

/// @brief Assert that a condition is true, and call a user-defined assert handler if it isn't. This
/// is an application-level convenience macro which should only be used in application code, and
/// requires the application developer to implement the `jcfw_platform_on_assert` hook.
/// @param _cond The condition to assert.
/// @param _msg_fmt The format of the message to log.
/// @param __VA_ARGS__ Arguments for the log message.
#define JCFW_ASSERT(_cond, _msg_fmt, ...)                                                          \
    do                                                                                             \
    {                                                                                              \
        if (JCFW_UNLIKELY(!(_cond)))                                                               \
        {                                                                                          \
            jcfw_platform_on_assert(__FILE__, __LINE__, (_msg_fmt), ##__VA_ARGS__);                \
        }                                                                                          \
    } while (0)

/// @brief Assert that a condition is true, and return (with a value, optionally) if it isn't. This
/// macro is safe to use for application code and library code.
/// @param _cond The condition to assert.
/// @param __VA_ARGS__ Optional; The value to return.
#define JCFW_ASSERT_RET(_cond, ...)                                                                \
    do                                                                                             \
    {                                                                                              \
        if (JCFW_UNLIKELY(!(_cond)))                                                               \
        {                                                                                          \
            return __VA_ARGS__;                                                                    \
        }                                                                                          \
    } while (0)

#define JCFW_CHECK(_cond, _format, ...)                                                            \
    do                                                                                             \
    {                                                                                              \
        if (JCFW_UNLIKELY(!(_cond)))                                                               \
        {                                                                                          \
            JCFW_TRACELN("CHECK", "Check failed: %s. ", #_cond);                                   \
            JCFW_TRACELN("CHECK", _format, ##__VA_ARGS__);                                         \
        }                                                                                          \
    } while (0)

#define JCFW_NEWASSERT(_cond, _format, ...)
#define JCFW_CRASH(_format, ...)
#define JCFW_RETURN_VAL_IF_FALSE(_cond, _val, _format, ...)
#define JCFW_RETURN_IF_FALSE(_cond, _format, ...)

#endif //  __JCFW_UTIL_ASSERT_H__
