#include "jcfw/util/checksum.h"

#define JCFW_CHECKSUM_TRUNC_POLY 0x1021 // x^16 + x^12 + x^5 + 1
#define JCFW_CHECKSUM_SEED       0xFFFF // initial remainder/seed
#define JCFW_CHECKSUM_XOR        0xFFFF // final XOR value

uint16_t jcfw_crc16(const uint8_t *data, size_t size)
{
    uint16_t crc = JCFW_CHECKSUM_SEED;

    for (size_t i = 0; i < size; i++)
    {
        uint8_t b = data[i];
        for (size_t j = 0; j < 8; j++)
        {
            if (((crc & 0x8000) >> 8) ^ (b & 0x80))
            {
                crc <<= 1;
                crc ^= JCFW_CHECKSUM_TRUNC_POLY;
            }
            else
            {
                crc <<= 1;
            }
            b <<= 1;
        }
    }

    return crc ^ JCFW_CHECKSUM_XOR;
}
