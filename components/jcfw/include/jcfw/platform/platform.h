#ifndef __JFCW_PLATOFRM_PLATFORM_H__
#define __JFCW_PLATOFRM_PLATFORM_H__

#include "jcfw/detail/common.h"
#include "jcfw/util/result.h"

jcfw_result_e jcfw_platform_init(void);

bool jcfw_platform_trace_validate(const char *tag);
void jcfw_platform_trace_putc(char c, bool flush);

#endif // __JFCW_PLATOFRM_PLATFORM_H__
