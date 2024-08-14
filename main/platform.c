#include "jcfw/platform/platform.h"

#include <stdio.h>

jcfw_result_e jcfw_platform_init(void)
{
    // ...

    return JCFW_RESULT_OK;
}

bool jcfw_platform_trace_validate(const char *tag)
{
    // ...

    return true;
}

void jcfw_platform_trace_putc(char c, bool flush)
{
    putc(c, stdout);

    if (flush)
    {
        fflush(stdout);
    }
}
