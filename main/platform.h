#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "driver/i2c_master.h"
#include "jcfw/driver/als/ltr303.h"

// #define CLI_UART_NUM UART_NUM_0

extern jcfw_ltr303_t g_ltr303;
extern bool          g_is_als_data_ready;
// extern QueueHandle_t g_cli_uart_event_queue;

#endif // __PLATFORM_H__
