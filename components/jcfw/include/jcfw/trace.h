#ifndef __JCFW_TRACE_H__
#define __JCFW_TRACE_H__

#include "jcfw/detail/common.h"
#include "jcfw/platform/platform.h"

#define _JCFW_TRACE_COLOR_DEFAULT "\033[39m"
#define _JCFW_TRACE_COLOR_RED     "\033[91m"
#define _JCFW_TRACE_COLOR_GRAY    "\033[90m"
#define _JCFW_TRACE_COLOR_GREEN   "\033[92m"
#define _JCFW_TRACE_COLOR_MAGENTA "\033[95m"
#define _JCFW_TRACE_COLOR_YELLOW  "\033[93m"

typedef enum
{
    JCFW_TRACE_LEVEL_DEBUG = 0,
    JCFW_TRACE_LEVEL_INFO,
    JCFW_TRACE_LEVEL_WARN,
    JCFW_TRACE_LEVEL_ERROR,
    JCFW_TRACE_LEVEL_NOTIFICATION,
} jcfw_trace_level_e;

/// @brief Initialize the trace module.
/// @param putchar_func The function that the trace module will use for output.
/// @param putchar_arg Optional; The argument to pass to the output function.
void jcfw_trace_init(jcfw_platform_putchar_f putchar_func, void *putchar_arg);

/// @brief Set the level for output from the trace module.
/// @param level The level to set for the module.
void jcfw_trace_set_level(jcfw_trace_level_e level);

// -------------------------------------------------------------------------------------------------

void _jcfw_trace_generic(
    const char        *tag,
    jcfw_trace_level_e level,
    const char        *file,
    int                line,
    const char        *color,
    const char        *prefix,
    const char        *postfix,
    const char        *format,
    ...);

void _jcfw_tracehex_generic(
    const char        *tag,
    jcfw_trace_level_e level,
    const char        *file,
    int                line,
    const char        *color,
    const char        *prefix,
    const void        *data,
    size_t             size,
    const char        *user_prefix);

#define _JCFW_TRACE_IMPL(_tag, _level, _color, _prefix, _postfix, ...)                             \
    _jcfw_trace_generic(_tag, _level, __FILE__, __LINE__, _color, _prefix, _postfix, ##__VA_ARGS__)

#define _JCFW_TRACEHEX_IMPL(_tag, _level, _color, _prefix, _data, _size, _user_prefix)             \
    _jcfw_tracehex_generic(                                                                        \
        _tag, _level, __FILE__, __LINE__, _color, _prefix, _data, _size, _user_prefix)

// -------------------------------------------------------------------------------------------------

/// @brief Print a trace log.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACE_DEBUG(_tag, ...)                                                                \
    _JCFW_TRACE_IMPL(_tag, JCFW_TRACE_LEVEL_DEBUG, _JCFW_TRACE_COLOR_GRAY, "D", "", ##__VA_ARGS__)

/// @brief Print a trace log.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACE_INFO(_tag, ...)                                                                 \
    _JCFW_TRACE_IMPL(_tag, JCFW_TRACE_LEVEL_INFO, _JCFW_TRACE_COLOR_GREEN, "I", "", ##__VA_ARGS__)

/// @brief Print a trace log.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACE_WARN(_tag, ...)                                                                 \
    _JCFW_TRACE_IMPL(_tag, JCFW_TRACE_LEVEL_WARN, _JCFW_TRACE_COLOR_YELLOW, "W", "", ##__VA_ARGS__)

/// @brief Print a trace log.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACE_ERROR(_tag, ...)                                                                \
    _JCFW_TRACE_IMPL(_tag, JCFW_TRACE_LEVEL_ERROR, _JCFW_TRACE_COLOR_RED, "E", "", ##__VA_ARGS__)

/// @brief Print a trace log.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACE_NOTIFICATION(_tag, ...)                                                         \
    _JCFW_TRACE_IMPL(                                                                              \
        _tag, JCFW_TRACE_LEVEL_NOTIFICATION, _JCFW_TRACE_COLOR_MAGENTA, "!", "", ##__VA_ARGS__)

/// @brief Print a trace log.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACE(_tag, ...) JCFW_TRACE_INFO(_tag, ##__VA_ARGS__)

// -------------------------------------------------------------------------------------------------

/// @brief Print a trace log with a trailing newline.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACELN_DEBUG(_tag, ...)                                                              \
    _JCFW_TRACE_IMPL(_tag, JCFW_TRACE_LEVEL_DEBUG, _JCFW_TRACE_COLOR_GRAY, "D", "\n", ##__VA_ARGS__)

/// @brief Print a trace log with a trailing newline.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACELN_INFO(_tag, ...)                                                               \
    _JCFW_TRACE_IMPL(_tag, JCFW_TRACE_LEVEL_INFO, _JCFW_TRACE_COLOR_GREEN, "I", "\n", ##__VA_ARGS__)

/// @brief Print a trace log with a trailing newline.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACELN_WARN(_tag, ...)                                                               \
    _JCFW_TRACE_IMPL(                                                                              \
        _tag, JCFW_TRACE_LEVEL_WARN, _JCFW_TRACE_COLOR_YELLOW, "W", "\n", ##__VA_ARGS__)

/// @brief Print a trace log with a trailing newline.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACELN_ERROR(_tag, ...)                                                              \
    _JCFW_TRACE_IMPL(_tag, JCFW_TRACE_LEVEL_ERROR, _JCFW_TRACE_COLOR_RED, "E", "\n", ##__VA_ARGS__)

/// @brief Print a trace log with a trailing newline.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACELN_NOTIFICATION(_tag, ...)                                                       \
    _JCFW_TRACE_IMPL(                                                                              \
        _tag, JCFW_TRACE_LEVEL_NOTIFICATION, _JCFW_TRACE_COLOR_MAGENTA, "!", "\n", ##__VA_ARGS__)

/// @brief Print a trace log with a trailing newline.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACELN(_tag, ...) JCFW_TRACELN_INFO(_tag, ##__VA_ARGS__)

// -------------------------------------------------------------------------------------------------

/// @brief Print a series of trace logs which print the contents of some memory in a hexdump-like
/// format.
/// @param _tag The tag string to print the trace log under.
/// @param _data The data to log.
/// @param _size The size of the data to log.
/// @param _prefix Optional; The prefix to use for each line of output.
#define JCFW_TRACEHEX_DEBUG(_tag, _data, _size, _prefix)                                           \
    _JCFW_TRACEHEX_IMPL(                                                                           \
        _tag, JCFW_TRACE_LEVEL_DEBUG, _JCFW_TRACE_COLOR_GRAY, "D", _data, _size, _prefix)

/// @brief Print a series of trace logs which print the contents of some memory in a hexdump-like
/// format.
/// @param _tag The tag string to print the trace log under.
/// @param _data The data to log.
/// @param _size The size of the data to log.
/// @param _prefix Optional; The prefix to use for each line of output.
#define JCFW_TRACEHEX_INFO(_tag, _data, _size, _prefix)                                            \
    _JCFW_TRACEHEX_IMPL(                                                                           \
        _tag, JCFW_TRACE_LEVEL_INFO, _JCFW_TRACE_COLOR_GREEN, "I", _data, _size, _prefix)

/// @brief Print a series of trace logs which print the contents of some memory in a hexdump-like
/// format.
/// @param _tag The tag string to print the trace log under.
/// @param _data The data to log.
/// @param _size The size of the data to log.
/// @param _prefix Optional; The prefix to use for each line of output.
#define JCFW_TRACEHEX_WARN(_tag, _data, _size, _prefix)                                            \
    _JCFW_TRACEHEX_IMPL(                                                                           \
        _tag, JCFW_TRACE_LEVEL_WARN, _JCFW_TRACE_COLOR_YELLOW, "W", _data, _size, _prefix)

/// @brief Print a series of trace logs which print the contents of some memory in a hexdump-like
/// format.
/// @param _tag The tag string to print the trace log under.
/// @param _data The data to log.
/// @param _size The size of the data to log.
/// @param _prefix Optional; The prefix to use for each line of output.
#define JCFW_TRACEHEX_ERROR(_tag, _data, _size, _prefix)                                           \
    _JCFW_TRACEHEX_IMPL(                                                                           \
        _tag, JCFW_TRACE_LEVEL_ERROR, _JCFW_TRACE_COLOR_RED, "E", _data, _size, _prefix)

/// @brief Print a series of trace logs which print the contents of some memory in a hexdump-like
/// format.
/// @param _tag The tag string to print the trace log under.
/// @param _data The data to log.
/// @param _size The size of the data to log.
/// @param _prefix Optional; The prefix to use for each line of output.
#define JCFW_TRACEHEX_NOTIFICATION(_tag, _data, _size, _prefix)                                    \
    _JCFW_TRACEHEX_IMPL(                                                                           \
        _tag,                                                                                      \
        JCFW_TRACE_LEVEL_NOTIFICATION,                                                             \
        _JCFW_TRACE_COLOR_MAGENTA,                                                                 \
        "!",                                                                                       \
        _data,                                                                                     \
        _size,                                                                                     \
        _prefix)

/// @brief Print a series of trace logs which print the contents of some memory in a hexdump-like
/// format.
/// @param _tag The tag string to print the trace log under.
/// @param _data The data to log.
/// @param _size The size of the data to log.
/// @param _prefix Optional; The prefix to use for each line of output.
#define JCFW_TRACEHEX(_tag, _data, _size, _prefix) JCFW_TRACEHEX_INFO(_tag, _data, _size, _prefix)

#endif // __JCFW_TRACE_H__
