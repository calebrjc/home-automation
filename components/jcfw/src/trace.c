#include "jcfw/trace.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h> // temporary!

#include "jcfw/platform/platform.h"
#include "jcfw/util/assert.h"

// TODO(Caleb): Do I need to lock this module?

static void _jcfw_trace_puts(const char *s);
static void _jcfw_trace_printf(const char *format, ...);

void _jcfw_trace_impl(const char *tag, const char *file, int line, const char *format, ...)
{
    if (!jcfw_platform_trace_validate(tag))
    {
        return;
    }

    _jcfw_trace_printf("[%-*s] ", JCFW_TRACE_MAX_TAG_LEN, tag);

    if (file)
    {
        _jcfw_trace_printf("%s:%d - ", file, line);
    }

    va_list args;
    va_start(args, format);

    size_t size = (size_t)vsnprintf(NULL, 0, format, args) + 1;
    char   buffer[size];
    vsnprintf(buffer, size, format, args);

    va_end(args);

    buffer[size - 1] = '\0';
    _jcfw_trace_puts(buffer);
}

void _jcfw_traceln_impl(const char *tag, const char *file, int line, const char *format, ...)
{
    if (!jcfw_platform_trace_validate(tag))
    {
        return;
    }

    _jcfw_trace_printf("[%-*s] ", JCFW_TRACE_MAX_TAG_LEN, tag);

    if (file)
    {
        _jcfw_trace_printf("%s:%d - ", file, line);
    }

    va_list args;
    va_start(args, format);

    size_t size = (size_t)vsnprintf(NULL, 0, format, args) + 1;
    char   buffer[size];
    vsnprintf(buffer, size, format, args);

    va_end(args);

    buffer[size - 1] = '\0';
    _jcfw_trace_printf("%s\n", buffer);
}

void JCFW_TRACEHEX(const char *tag, const void *data, size_t size, const char *prefix)
{
    JCFW_ASSERT_RET(tag && data && size);

    for (size_t i = 0; i < size; i += 16)
    {
        _jcfw_trace_printf("[%-*s] ", JCFW_TRACE_MAX_TAG_LEN, tag);

        if (prefix)
        {
            _jcfw_trace_printf("%s ", prefix);
        }

        _jcfw_trace_printf("- %04zx: ", i);

        for (size_t j = 0; j < 16; ++j)
        {
            if (i + j < size)
            {
                _jcfw_trace_printf("%02x ", ((unsigned char *)data)[i + j]);
            }
            else
            {
                _jcfw_trace_printf("   ");
            }
        }

        _jcfw_trace_printf(" |");
        for (size_t j = 0; j < 16; ++j)
        {
            if (i + j < size)
            {
                char ch = ((char *)data)[i + j];
                _jcfw_trace_printf("%c", isprint((unsigned char)ch) ? ch : '.');
            }
            else
            {
                _jcfw_trace_printf(" ");
            }
        }
        _jcfw_trace_printf("|\n");
    }
}

static void _jcfw_trace_puts(const char *s)
{
    JCFW_ASSERT_RET(s);

    while (*s)
    {
        jcfw_platform_trace_putc(*s, s[1] == '\0');
        s++;
    }
}

static void _jcfw_trace_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    size_t size = (size_t)vsnprintf(NULL, 0, format, args) + 1;
    char   buffer[size];
    vsnprintf(buffer, size, format, args);

    va_end(args);

    buffer[size - 1] = '\0';
    _jcfw_trace_puts(buffer);
}
