#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "driver/i2c_master.h"
#include "jcfw/driver/als/ltr303.h"

extern jcfw_ltr303_t g_ltr303;
extern bool          g_is_als_data_ready;

#endif // __PLATFORM_H__
