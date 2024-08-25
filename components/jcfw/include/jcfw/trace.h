#ifndef __JCFW_TRACE_H__
#define __JCFW_TRACE_H__

#include <stddef.h>

// TODO(Caleb): Make this work also with string variables

// ...
#define JCFW_TRACE(_tag, ...)   _jcfw_trace_impl(_tag, __FILE__, __LINE__, ##__VA_ARGS__)

// ...
#define JCFW_TRACELN(_tag, ...) _jcfw_traceln_impl(_tag, __FILE__, __LINE__, ##__VA_ARGS__)

#define JCFW_TRACEHEX(_tag, _data, _size, _prefix)                                                 \
    _jcfw_tracehex_impl(_tag, __FILE__, __LINE__, _data, _size, _prefix)

void _jcfw_trace_impl(const char *tag, const char *file, int line, const char *format, ...);
void _jcfw_traceln_impl(const char *tag, const char *file, int line, const char *format, ...);
void _jcfw_tracehex_impl(
    const char *tag, const char *file, int line, const void *data, size_t size, const char *prefix);

#endif // __JCFW_TRACE_H__
