#ifndef __JFCW_PLATOFRM_PLATFORM_H__
#define __JFCW_PLATOFRM_PLATFORM_H__

#include "jcfw/detail/common.h"
#include "jcfw/util/result.h"

/// @brief A function that can be used for output.
/// @param arg The argument to pass to this function.
/// @param c The character to output.
/// @param flush True when the calling code intends for any buffering to be flushed.
typedef void (*jcfw_platform_putchar_f)(void *arg, char c, bool flush);

/// @brief Initalize the platform layer. Manual hardware initalization to do with the platform layer
/// should be done here. This function should be called once from the main function.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_platform_init(void);

/// @brief Handle a failed assert. This function must be implemented to use any of the hard assert
/// macros. (see: jcfw/assert.h)
/// @param file The name of the file that the assert was triggered in.
/// @param line The line number in the file that the assert was triggered in.
/// @param format A `printf` message format.
/// @param ... The arguments for the message format.
void jcfw_platform_on_assert(const char *file, int line, const char *format, ...);

/// @brief Gvien a tag, return true if a trace with the tag should be output, and false otherwise.
/// @param tag The tag to evaluate.
/// @return True if a trace with the tag should be output, and false otherwise.
bool jcfw_platform_trace_validate(const char *tag);

/// @brief Delay (block) execution for some number of milliseconds.
/// @param delay_ms The amount of time to block for.
void jcfw_platform_delay_ms(uint32_t delay_ms);

/// @brief Perform an I2C master read from a "memory address" using the given argument.
/// @param arg The argument to use for the read. Use this for platform-dependent arguments.
/// @param mem_addr The "memory address" to read from the device at.
/// @param mem_addr_size The size of the "memory address".
/// @param o_data Required. The data returned by the read.
/// @param data_size The size of the buffer for the returned data. This call is expected to return
/// this much data.
/// @param timeout_ms The maximum timeout of the I2C operation.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_platform_i2c_mstr_mem_read(
    void          *arg,
    const uint8_t *mem_addr,
    size_t         mem_addr_size,
    uint8_t       *o_data,
    size_t         data_size,
    uint32_t       timeout_ms);

/// @brief Perform an I2C master write from a "memory address" using the given argument.
/// @param arg The argument to use for the write. Use this for platform-dependent arguments.
/// @param mem_addr The "memory address" to write to the device at.
/// @param mem_addr_size The size of the "memory address".
/// @param data The data to write to the "memory address".
/// @param data_size The size of the data to write.
/// @param timeout_ms The maximum timeout of the I2C operation.
/// @return JCFW_RESULT_OK if the operation is successful, or an error code otherwise.
jcfw_result_e jcfw_platform_i2c_mstr_mem_write(
    void          *arg,
    const uint8_t *mem_addr,
    size_t         mem_addr_size,
    const uint8_t *data,
    size_t         data_size,
    uint32_t       timeout_ms);

#endif // __JFCW_PLATOFRM_PLATFORM_H__
