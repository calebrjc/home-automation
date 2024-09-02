#include "driver/uart.h"

#include "jcfw/platform/wifi.h"
#include "jcfw/trace.h"
#include "jcfw/util/assert.h"

#include "cli.h"
#include "platform.h"
#include "util.h"

#define TRACE_TAG "MAIN"

void app_main(void)
{
    jcfw_result_e err;

    err = jcfw_platform_init();
    JCFW_ASSERT(err == JCFW_RESULT_OK, "error: Unable to execute platform initialization");

    jcfw_trace_init(util_putchar, NULL);

    err = jcfw_wifi_init();
    JCFW_ASSERT(err == JCFW_RESULT_OK, "Unable to initialize WIFI");
    JCFW_TRACELN_INFO(TRACE_TAG, "WIFI has been initialized");

    err = jcfw_wifi_sta_connect("Onions", "shallot13");
    JCFW_ASSERT(err == JCFW_RESULT_OK, "Unable to connect to the network");
    JCFW_TRACELN_INFO(TRACE_TAG, "WIFI has been connected");

    bool cli_inited = cli_init();
    JCFW_ASSERT(cli_inited, "Unable to initialize the CLI task");
    xTaskCreate(cli_run, "APP-CLI", 2048, NULL, tskIDLE_PRIORITY + 5, NULL);
}
