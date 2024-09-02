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

    err = jcfw_wifi_init();
    JCFW_ASSERT(err == JCFW_RESULT_OK, "Unable to initialize WIFI");
    JCFW_TRACELN_INFO(TRACE_TAG, "WIFI has been initialized");

    err = jcfw_wifi_sta_connect("*****", "*****");
    JCFW_ASSERT(err == JCFW_RESULT_OK, "Unable to connect to the network");
    JCFW_TRACELN_INFO(TRACE_TAG, "WIFI has been connected");

    bool cli_inited = cli_init();
    JCFW_ASSERT(cli_inited, "Unable to initialize the CLI task");
    xTaskCreate(cli_run, "APP-CLI", 4096, NULL, tskIDLE_PRIORITY + 5, NULL);

    // -------------------------------------------------------------------------

    ip_address_t server_addr = {0};
    const char  *MSG_FORMAT  = "{\"als\": %f}";

    int sock = create_socket("10.0.0.141", "5000", &server_addr, 3);
    JCFW_ASSERT(sock >= 0, "Unable to create the client socket");

    err = jcfw_ltr303_set_mode(&g_ltr303, JCFW_LTR303_MODE_ACTIVE);
    JCFW_ASSERT(sock >= 0, "Unable to start the LTR303");

    while (1)
    {
        uint16_t      ch0         = 0x0000;
        uint16_t      ch1         = 0x0000;
        uint8_t       gain_factor = 0;
        jcfw_result_e err         = jcfw_ltr303_read(&g_ltr303, &ch0, &ch1, &gain_factor);
        if (err != JCFW_RESULT_OK || gain_factor == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        else
        {
            float visible_light = JCFW_CLAMP((float)(ch0 - ch1), 0, 64000) / gain_factor;

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
        }

        JCFW_TRACELN_INFO(TRACE_TAG, "Sent packet to server");
        vTaskDelay(pdMS_TO_TICKS(1000));
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

    if ((rv = getaddrinfo(addr, port, &hints, &servinfo)) != 0)
    {
        JCFW_TRACELN_ERROR(TRACE_TAG, "getaddrinfo failed; rv %d\n", rv);
        return -1;
    }

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

    if (p == NULL)
    {
        JCFW_TRACELN_ERROR(TRACE_TAG, "error: Failed to create a socket\n");
        return -1;
    }

    if (timeout_sec)
    {
        struct timeval timeout;
        timeout.tv_sec  = timeout_sec;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
    }

    memcpy(&o_remote_addr->data, p->ai_addr, p->ai_addrlen);
    o_remote_addr->len = p->ai_addrlen;

    freeaddrinfo(servinfo);

    return sock;
}
