#ifndef __JFCW_PLATOFRM_PLATFORM_H__
#define __JFCW_PLATOFRM_PLATFORM_H__

#include "jcfw/detail/common.h"
#include "jcfw/util/result.h"

/// @brief Initalize the platform layer. Manual hardware initalization to do with the platform layer
/// should be done here. This function should be called once from the main function.
/// @return JCFW_RESULT_OK if initialization is successful, or an error code otherwise.
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

/// @brief Write a character to the trace output stream.
/// @param c The character to write.
/// @param flush True if a theoretical "buffer" should be "flushed" after writing the character.
/// Useful if the implementation takes advantage of buffering.
void jcfw_platform_trace_putc(char c, bool flush);

void jcfw_platform_delay_ms(uint32_t delay_ms);

jcfw_result_e jcfw_platform_i2c_mstr_mem_read(
    void          *arg,
    const uint8_t *mem_addr,
    size_t         mem_addr_size,
    uint8_t       *o_data,
    size_t         data_size,
    uint32_t       timeout_ms);

jcfw_result_e jcfw_platform_i2c_mstr_mem_write(
    void          *arg,
    const uint8_t *mem_addr,
    size_t         mem_addr_size,
    const uint8_t *data,
    size_t         data_size,
    uint32_t       timeout_ms);

#endif // __JFCW_PLATOFRM_PLATFORM_H__
