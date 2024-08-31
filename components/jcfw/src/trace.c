#include "jcfw/trace.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "jcfw/platform/platform.h"
#include "jcfw/util/assert.h"
#include "jcfw/util/math.h"

// TODO(Caleb): Do I need to lock this module?

// -------------------------------------------------------------------------------------------------

static jcfw_platform_putchar_f s_putchar     = NULL;
static void                   *s_putchar_arg = NULL;
static jcfw_trace_level_e      s_level       = JCFW_TRACE_LEVEL_DEBUG;

// -------------------------------------------------------------------------------------------------

static void _jcfw_trace_puts(const char *s);
static void _jcfw_trace_printf(const char *format, ...);

// -------------------------------------------------------------------------------------------------

void jcfw_trace_init(jcfw_platform_putchar_f putchar_func, void *putchar_arg)
{
    s_putchar     = putchar_func;
    s_putchar_arg = putchar_arg;
}

void jcfw_trace_set_level(jcfw_trace_level_e level)
{
    s_level = JCFW_CLAMP(level, JCFW_TRACE_LEVEL_DEBUG, JCFW_TRACE_LEVEL_NOTIFICATION);
}

void _jcfw_trace_generic(
    const char        *tag,
    jcfw_trace_level_e level,
    const char        *file,
    int                line,
    const char        *color,
    const char        *prefix,
    const char        *postfix,
    const char        *format,
    ...)
{
    JCFW_RETURN_IF_FALSE(level >= s_level && jcfw_platform_trace_validate(tag));

    _jcfw_trace_printf("%s%s [%s] ", color, prefix, tag);

    if (file)
    {
        _jcfw_trace_printf("%s:%d - ", basename(file), line);
    }

    va_list args;
    va_start(args, format);

    size_t size = (size_t)vsnprintf(NULL, 0, format, args) + 1;
    char   buffer[size];
    vsnprintf(buffer, size, format, args);

    va_end(args);

    buffer[size - 1] = '\0';
    _jcfw_trace_printf("%s%s%s", buffer, postfix, _JCFW_TRACE_COLOR_DEFAULT);
}

void _jcfw_tracehex_generic(
    const char        *tag,
    jcfw_trace_level_e level,
    const char        *file,
    int                line,
    const char        *color,
    const char        *prefix,
    const void        *data,
    size_t             size,
    const char        *user_prefix)
{
    JCFW_RETURN_IF_FALSE(tag && data && size);
    JCFW_RETURN_IF_FALSE(level >= s_level && jcfw_platform_trace_validate(tag));

    for (size_t i = 0; i < size; i += 16)
    {
        _jcfw_trace_printf("%s%s [%s] ", color, prefix, tag);

        if (file)
        {
            _jcfw_trace_printf("%s:%d - ", basename(file), line);
        }

        if (user_prefix)
        {
            _jcfw_trace_printf("%s - ", user_prefix);
        }

        _jcfw_trace_printf("%04zx: ", i);

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

    _jcfw_trace_puts(_JCFW_TRACE_COLOR_DEFAULT);
}

static void _jcfw_trace_puts(const char *s)
{
    JCFW_RETURN_IF_FALSE(s && s_putchar);

    while (*s)
    {
        s_putchar(s_putchar_arg, *s, s[1] == '\0');
        s++;
    }
}

static void _jcfw_trace_printf(const char *format, ...)
{
    JCFW_RETURN_IF_FALSE(format);

    va_list args;
    va_start(args, format);

    size_t size = (size_t)vsnprintf(NULL, 0, format, args) + 1;
    char   buffer[size];
    vsnprintf(buffer, size, format, args);

    va_end(args);

    buffer[size - 1] = '\0';
    _jcfw_trace_puts(buffer);
}
