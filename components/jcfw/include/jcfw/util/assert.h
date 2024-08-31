#ifndef __JCFW_UTIL_ASSERT_H__
#define __JCFW_UTIL_ASSERT_H__

#include "jcfw/platform/compiler.h"
#include "jcfw/platform/platform.h"
#include "jcfw/trace.h"

/// @brief Trigger a crash with an error message. Requires `jcfw_platform_crash()` to be defined.
/// @param _format The `printf` format of the message.
/// @param ... The arguments for the message.
#define JCFW_CRASH(_format, ...)                                                                   \
    do                                                                                             \
    {                                                                                              \
        JCFW_TRACELN_ERROR("JCFW-CRASH", "Crash triggered - " _format, ##__VA_ARGS__);             \
        jcfw_platform_crash();                                                                     \
    } while (0)

/// @brief Return from a function (optionally with a value) if the given condition is false.
/// @note Think about the condition as an assert condition.
/// @param _cond The condition to check.
/// @param ... Optional; The value to return if the condition is false.
#define JCFW_RETURN_IF_FALSE(_cond, ...)
    do                                                                                             \
    {                                                                                              \
        if (JCFW_UNLIKELY(!(_cond)))                                                               \
        {                                                                                          \
            return __VA_ARGS__;                                                                    \
        }                                                                                          \
    } while (0)

/// @brief Return from a function (optionally with a value) if the given condition is true.
/// @param _cond The condition to check.
/// @param ... Optional; The value to return if the condition is true.
#define JCFW_RETURN_IF_TRUE(_cond, ...) JCFW_RETURN_IF_FALSE(!(_cond), ##__VA_ARGS__)

/// @brief If the given condition is false, print an error trace with the given format and arguments
/// and return (optionally with a value).
/// @param _cond The condition to check.
/// @param _retval Optional; The value to return if the condition is false.
/// @param _format The message format.
/// @param ... The arguments for the message format.
#define JCFW_ERROR_IF_FALSE(_cond, _retval, _format, ...)                                          \
    do                                                                                             \
    {                                                                                              \
        if (JCFW_UNLIKELY(!(_cond)))                                                               \
        {                                                                                          \
            JCFW_TRACELN_ERROR("JCFW-CHECK", "Early return - error: " _format, ##__VA_ARGS__);     \
            return _retval;                                                                        \
        }                                                                                          \
    } while (0)

/// @brief If the given condition is true, print an error trace with the given format and arguments
/// and return (optionally with a value).
/// @param _cond The condition to check.
/// @param _retval Optional; The value to return if the condition is true.
/// @param _format The message format.
/// @param ... The arguments for the message format.
#define JCFW_ERROR_IF_TRUE(_cond, _retval, _format, ...)                                           \
    JCFW_ERROR_IF_FALSE(!(_cond), _retval, _format, ##__VA_ARGS__)

/// @brief If the given condition is false, print an error trace with the given format and
/// arguments.
/// @note Think of the condition as an assert condition.
/// @param _cond The condition to check.
/// @param _format The message format.
/// @param ... The arguments for the message format.
#define JCFW_CHECK(_cond, _format, ...)                                                            \
    do                                                                                             \
    {                                                                                              \
        if (JCFW_UNLIKELY(!(_cond)))                                                               \
        {                                                                                          \
            JCFW_TRACELN_ERROR(                                                                    \
                "JCFW-CHECK", "Check failed (%s) - " _format, #_cond, ##__VA_ARGS__);              \
        }                                                                                          \
    } while (0)

#ifdef JCFW_DEBUG
/// @brief If the given condition is false, print an error trace with the given format and
/// arguments.
/// @note Think of the condition as an assert condition.
/// @note This condition will not be evaluated in non-debug builds.
/// @param _cond The condition to check.
/// @param _format The message format.
/// @param ... The arguments for the message format.
#define JCFW_DCHECK(_cond, _format, ...) JCFW_CHECK(_cond, _format, ##__VA_ARGS__)
#else
#define JCFW_DCHECK(...)
#endif

/// @brief If the given condition is false, print an error trace with the given format and
/// arguments, then crash the processor. Requires `jcfw_platform_crash()` to be defined.
/// @note Think of the condition as an assert condition.
/// @param _cond The condition to check.
/// @param _format The message format.
/// @param ... The arguments for the message format.
#define JCFW_ASSERT(_cond, _format, ...)                                                           \
    do                                                                                             \
    {                                                                                              \
        if (JCFW_UNLIKELY(!(_cond)))                                                               \
        {                                                                                          \
            JCFW_TRACELN_ERROR(                                                                    \
                "JCFW-ASSERT", "Assert failed (%s) - " _format, #_cond, ##__VA_ARGS__);            \
            jcfw_platform_crash();                                                                 \
        }                                                                                          \
    } while (0)

#ifdef JCFW_DEBUG
/// @brief If the given condition is false, print an error trace with the given format and
/// arguments, then crash the processor. Requires `jcfw_platform_crash()` to be defined.
/// @note Think of the condition as an assert condition.
/// @note This condition will not be evaluated in non-debug builds.
/// @param _cond The condition to check.
/// @param _format The message format.
/// @param ... The arguments for the message format.
#define JCFW_DASSERT(_cond, _format, ...) JCFW_ASSERT(_cond, _format, ##__VA_ARGS__)
#else
#define JCFW_DASSERT(...)
#endif
#endif //  __JCFW_UTIL_ASSERT_H__
