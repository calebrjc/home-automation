#include "jcfw/util/string.h"

static char _jcfw_hex_to_char(char c)
{
    if ('0' <= c && c <= '9')
    {
        return c - '0';
    }
    else if ('a' <= c && c <= 'f')
    {
        return c + 10 - 'a';
    }
    else if ('A' <= c && c <= 'F')
    {
        return c + 10 - 'A';
    }

    return (char)-1;
}

size_t jcfw_hex_to_binary(uint8_t *dest, const char *src, size_t size)
{
    size_t length;
    int    high, low;
    size_t n = 0;

    if (src == NULL || dest == NULL || size == 0)
        return (char)-1;

    length = strlen(src);
    length -= length % 2;
    if (size < length / 2)
    {
        length = size * 2;
    }

    memset(dest, 0, size);

    for (size_t i = 0; i < length; i += 2, ++n)
    {
        high = _jcfw_hex_to_char(*src);
        if (high < 0)
        {
            return (char)-2;
        }
        src++;

        low = _jcfw_hex_to_char(*src);
        if (low < 0)
        {
            return -3;
        }
        src++;

        dest[n] = high << 4 | low;
    }

    return n;
}
