#include "driver/uart.h"

// TODO(Caleb): Move these to net lib
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "jcfw/platform/wifi.h"
#include "jcfw/trace.h"
#include "jcfw/util/assert.h"
#include "jcfw/util/math.h"

#include "cli.h"
#include "platform.h"
#include "util.h"

#define TRACE_TAG "MAIN"

typedef struct
{
    struct sockaddr_storage data;
    socklen_t               len;
} ip_address_t;

int create_socket(
    const char *addr, const char *port, ip_address_t *o_remote_addr, uint32_t timeout_sec);

void app_main(void)
{
    jcfw_result_e err;

    err = jcfw_platform_init();
    JCFW_ASSERT(err == JCFW_RESULT_OK, "error: Unable to execute platform initialization");

    jcfw_trace_init(util_putchar, NULL);
    JCFW_TRACELN_ERROR("MAIN", "Here's an error message!");
    JCFW_TRACELN_WARN("MAIN", "Here's an warning message!");
    JCFW_TRACELN("MAIN", "Here's an info message!");
    JCFW_TRACELN_DEBUG("MAIN", "Here's a debug message!");
    JCFW_TRACELN_NOTIFICATION("MAIN", "Here's a notification message!");

    err = jcfw_wifi_init();
    JCFW_ASSERT(err == JCFW_RESULT_OK, "Unable to initialize WIFI");
    JCFW_TRACELN_INFO(TRACE_TAG, "WIFI has been initialized");

    err = jcfw_wifi_sta_connect("**********", "**********");
    JCFW_ASSERT(err == JCFW_RESULT_OK, "Unable to connect to the network");
    JCFW_TRACELN_INFO(TRACE_TAG, "WIFI has been connected");

    JCFW_ASSERT(cli_init(), "error: Unable to initialize the CLI task");
    JCFW_ASSERT(
        xTaskCreate(cli_run, "APP-CLI", 4096, NULL, tskIDLE_PRIORITY + 5, NULL),
        "error: Unable to start the CLI task");

    // -------------------------------------------------------------------------

    ip_address_t server_addr = {0};
    const char  *MSG_FORMAT  = "{\"als\": %f}";

    int sock = create_socket("***.***.***.***", "5000", &server_addr, 3);
    JCFW_ASSERT(sock >= 0, "Unable to create the client socket");

    err = jcfw_ltr303_set_mode(&g_ltr303, JCFW_LTR303_MODE_ACTIVE);
    JCFW_ASSERT(sock >= 0, "error: Unable to start the LTR303");

    while (1)
    {
        if (g_is_als_data_ready)
        {
            uint16_t      ch0         = 0x0000;
            uint16_t      ch1         = 0x0000;
            uint8_t       gain_factor = 0;
            jcfw_result_e err         = jcfw_ltr303_read(&g_ltr303, &ch0, &ch1, &gain_factor);
            if (err != JCFW_RESULT_OK)
            {
                JCFW_TRACE_ERROR(TRACE_TAG, "error: Unable to read ALS data\n");
            }
            else if (gain_factor == 0)
            {
                JCFW_TRACE_ERROR(TRACE_TAG, "error: Invalid gain read\n");
            }
            else
            {
                float visible_light = JCFW_CLAMP((float)(ch0 - ch1), 0, 64000) / gain_factor;
                JCFW_TRACE_DEBUG(TRACE_TAG, "ALS DATA: %f lux\n", visible_light);

                char data[32] = {0};
                snprintf(data, 32, MSG_FORMAT, visible_light);
                uint8_t bytes_sent = sendto(
                    sock,
                    data,
                    strnlen(data, 32),
                    0,
                    (struct sockaddr *)&server_addr.data,
                    server_addr.len);
                JCFW_ASSERT(bytes_sent > 0, "Unable to send data to the server");

                JCFW_TRACELN_INFO(TRACE_TAG, "Sent packet to server");
            }

            g_is_als_data_ready = false;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int create_socket(
    const char *addr, const char *port, ip_address_t *o_remote_addr, uint32_t timeout_sec)
{
    JCFW_RETURN_IF_FALSE(addr && port && o_remote_addr, -1);

    int             sock;
    struct addrinfo hints, *servinfo, *p;
    int             rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    rv = getaddrinfo(addr, port, &hints, &servinfo);
    JCFW_ERROR_IF_FALSE(rv == 0, -1, "getaddrinfo failed; rv %d", rv);

    // Loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            JCFW_TRACELN_ERROR(TRACE_TAG, "socket failed; errno %d\n", errno);
            continue;
        }

        break;
    }

    JCFW_ERROR_IF_FALSE(p != NULL, -1, "error: Failed to create a socket");

    if (timeout_sec)
    {
        struct timeval timeout;
        timeout.tv_sec  = timeout_sec;
        timeout.tv_usec = 0;
        JCFW_ERROR_IF_TRUE(
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0,
            -1,
            "error: Unable to set a timeout for the socket; errno %d",
            errno);
    }

    memcpy(&o_remote_addr->data, p->ai_addr, p->ai_addrlen);
    o_remote_addr->len = p->ai_addrlen;

    freeaddrinfo(servinfo);

    return sock;
}
