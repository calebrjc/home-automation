#ifndef __JCFW_UTIL_CHECKSUM_H__
#define __JCFW_UTIL_CHECKSUM_H__

#include "jcfw/detail/common.h"

/// @brief Return a 16-bit CRC of the given data.
/// @param data The data to create a CRC for.
/// @param size The size of the data.
/// @return A 16-bit CRC of the given data.
uint16_t jcfw_crc16(const uint8_t *data, size_t size);

#endif // __JCFW_UTIL_CHECKSUM_H__
