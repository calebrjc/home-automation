#include "util.h"

#include <stdio.h>

void util_putchar(void *arg, char c, bool flush)
{
    putc(c, stdout);

    if (flush)
    {
        fflush(stdout);
    }
}
