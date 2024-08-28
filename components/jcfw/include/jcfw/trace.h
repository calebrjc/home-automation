#ifndef __JCFW_TRACE_H__
#define __JCFW_TRACE_H__

#include <stddef.h>

#include "jcfw/platform/platform.h"

/// @brief Initialize the trace module.
/// @param putchar_func The function that the trace module will use for output.
/// @param putchar_arg Optional; The argument to pass to the output function.
void jcfw_trace_init(jcfw_platform_putchar_f putchar_func, void *putchar_arg);

/// @brief Print a trace log.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACE(_tag, ...)   _jcfw_trace_impl(_tag, __FILE__, __LINE__, ##__VA_ARGS__)

/// @brief Print a trace log with a trailing newline.
/// @param _tag The tag string to print the trace log under.
/// @param ... Printf-style arguments to format the trace message.
#define JCFW_TRACELN(_tag, ...) _jcfw_traceln_impl(_tag, __FILE__, __LINE__, ##__VA_ARGS__)

/// @brief Print a series of trace logs which print the contents of some memory in a hexdump-like
/// format.
/// @param _tag The tag string to print the trace log under.
/// @param _data The data to log.
/// @param _size The size of the data to log.
/// @param _prefix Optional; 
#define JCFW_TRACEHEX(_tag, _data, _size, _prefix)                                                 \
    _jcfw_tracehex_impl(_tag, __FILE__, __LINE__, _data, _size, _prefix)

void _jcfw_trace_impl(const char *tag, const char *file, int line, const char *format, ...);
void _jcfw_traceln_impl(const char *tag, const char *file, int line, const char *format, ...);
void _jcfw_tracehex_impl(
    const char *tag, const char *file, int line, const void *data, size_t size, const char *prefix);

#endif // __JCFW_TRACE_H__
