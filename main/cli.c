#include "cli.h"

// TODO(Caleb): JCFW OS
#include "freertos/FreeRTOS.h"

#include "jcfw/cli.h"
#include "jcfw/platform/wifi.h"
#include "jcfw/util/assert.h"
#include "jcfw/util/math.h"

#include "platform.h"
#include "util.h"

// -------------------------------------------------------------------------------------------------

static jcfw_cli_t s_cli = {0};

// -------------------------------------------------------------------------------------------------

static int als(jcfw_cli_t *cli, int argc, char **argv);

static int wifi(jcfw_cli_t *cli, int argc, char **argv);
static int wifi_status(jcfw_cli_t *cli, int argc, char **argv);
static int wifi_connect(jcfw_cli_t *cli, int argc, char **argv);
static int wifi_disconnect(jcfw_cli_t *cli, int argc, char **argv);
static int wifi_scan(jcfw_cli_t *cli, int argc, char **argv);

// -------------------------------------------------------------------------------------------------

const jcfw_cli_cmd_spec_t s_cmds[] = {
    {
        .name        = "als",
        .usage       = "usage: als <on|off>",
        .handler     = als,
        .num_subcmds = 0,
        .subcmds     = NULL,
    },
    {
        .name        = "wifi",
        .usage       = "usage: wifi <on|off>",
        .handler     = wifi,
        .num_subcmds = 4,
        .subcmds =
            (jcfw_cli_cmd_spec_t[]) {
                {
                    .name        = "status",
                    .usage       = "wifi status",
                    .handler     = wifi_status,
                    .num_subcmds = 0,
                    .subcmds     = NULL,
                },
                {
                    .name        = "connect",
                    .usage       = "wifi connect <ssid> <password>",
                    .handler     = wifi_connect,
                    .num_subcmds = 0,
                    .subcmds     = NULL,
                },
                {
                    .name        = "disconnect",
                    .usage       = "wifi disconnect",
                    .handler     = wifi_disconnect,
                    .num_subcmds = 0,
                    .subcmds     = NULL,
                },
                {
                    .name        = "scan",
                    .usage       = "wifi scan",
                    .handler     = wifi_scan,
                    .num_subcmds = 0,
                    .subcmds     = NULL,
                },
            },
    },
};

// -------------------------------------------------------------------------------------------------

static int als(jcfw_cli_t *cli, int argc, char **argv)
{
    const char *USAGE_MESSAGE        = "usage: als <on|off>\n";
    const char *ERROR_MESSAGE_FORMAT = "error: Unable to put the ALS in %s mode\n";

    if (argc != 2)
    {
        jcfw_cli_printf(cli, USAGE_MESSAGE);
        return EXIT_FAILURE;
    }

    jcfw_result_e err = JCFW_RESULT_ERROR;

    if (strncmp(argv[1], "on", 2) == 0)
    {
        err = jcfw_ltr303_set_mode(&g_ltr303, JCFW_LTR303_MODE_ACTIVE);
        if (err != JCFW_RESULT_OK)
        {
            jcfw_cli_printf(cli, ERROR_MESSAGE_FORMAT, "ACTIVE");
            return EXIT_FAILURE;
        }
    }
    else if (strncmp(argv[1], "off", 3) == 0)
    {
        err = jcfw_ltr303_set_mode(&g_ltr303, JCFW_LTR303_MODE_STANDBY);
        if (err != JCFW_RESULT_OK)
        {
            jcfw_cli_printf(cli, ERROR_MESSAGE_FORMAT, "STANDBY");
            return EXIT_FAILURE;
        }
    }
    else
    {
        jcfw_cli_printf(cli, USAGE_MESSAGE);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int wifi(jcfw_cli_t *cli, int argc, char **argv)
{
    const char *USAGE_MESSAGE        = "usage: wifi <on|off>\n";
    const char *NOOP_MESSAGE_FORMAT  = "WIFI is already %s\n";
    const char *ERROR_MESSAGE_FORMAT = "error: Unable to %s WIFI\n";

    if (argc != 2)
    {
        jcfw_cli_printf(cli, USAGE_MESSAGE);
        return EXIT_FAILURE;
    }

    jcfw_result_e err = JCFW_RESULT_ERROR;

    if (strncmp(argv[1], "on", 2) == 0)
    {
        if (jcfw_wifi_is_initialized())
        {
            jcfw_cli_printf(cli, NOOP_MESSAGE_FORMAT, "enabled");
            return EXIT_SUCCESS;
        }

        err = jcfw_wifi_init();
        if (err != JCFW_RESULT_OK)
        {
            jcfw_cli_printf(cli, ERROR_MESSAGE_FORMAT, "enable");
            return EXIT_FAILURE;
        }
    }
    else if (strncmp(argv[1], "off", 3) == 0)
    {
        if (!jcfw_wifi_is_initialized())
        {
            jcfw_cli_printf(cli, NOOP_MESSAGE_FORMAT, "disabled");
            return EXIT_SUCCESS;
        }

        err = jcfw_wifi_deinit();
        if (err != JCFW_RESULT_OK)
        {
            jcfw_cli_printf(cli, ERROR_MESSAGE_FORMAT, "disable");
            return EXIT_FAILURE;
        }
    }
    else
    {
        jcfw_cli_printf(cli, USAGE_MESSAGE);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int wifi_status(jcfw_cli_t *cli, int argc, char **argv)
{
    if (argc != 1)
    {
        jcfw_cli_printf(cli, "usage: wifi status\n");
        return EXIT_FAILURE;
    }

    const char *init_status = (jcfw_wifi_is_initialized()) ? "INITIALIZED" : "NOT INITIALIZED";
    const char *sta_conn_status =
        (jcfw_wifi_sta_is_connected()) ? "STA CONNECTED" : "STA NOT CONNECTED";

    jcfw_cli_printf(cli, "%s, %s\n", init_status, sta_conn_status);
    return EXIT_SUCCESS;
}

static int wifi_connect(jcfw_cli_t *cli, int argc, char **argv)
{
    if (argc < 2 && 3 > argc)
    {
        jcfw_cli_printf(cli, "usage: wifi connect <ssid> [password]\n");
        return EXIT_FAILURE;
    }

    if (!jcfw_wifi_is_initialized())
    {
        jcfw_cli_printf(cli, "WIFI is not enabled\n");
        return EXIT_FAILURE;
    }

    jcfw_cli_printf(cli, "Connecting to AP with SSID %s ", argv[1]);

    if (argc == 3)
    {
        jcfw_cli_printf(cli, "and password %s\n", argv[2]);
    }
    else
    {
        jcfw_cli_printf(cli, "and no password\n");
    }

    jcfw_result_e err = jcfw_wifi_sta_connect(argv[1], (argc == 3) ? argv[2] : NULL);
    return (err == JCFW_RESULT_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int wifi_disconnect(jcfw_cli_t *cli, int argc, char **argv)
{
    if (argc != 1)
    {
        jcfw_cli_printf(cli, "usage: wifi disconnect");
        return EXIT_FAILURE;
    }

    if (!jcfw_wifi_is_initialized())
    {
        jcfw_cli_printf(cli, "WIFI is not enabled\n");
        return EXIT_FAILURE;
    }

    jcfw_result_e err = jcfw_wifi_sta_disconnect();
    return (err == JCFW_RESULT_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int wifi_scan(jcfw_cli_t *cli, int argc, char **argv)
{
    if (argc != 1)
    {
        jcfw_cli_printf(cli, "usage: wifi scan");
        return EXIT_FAILURE;
    }

    if (!jcfw_wifi_is_initialized())
    {
        jcfw_cli_printf(cli, "WIFI is not enabled\n");
        return EXIT_FAILURE;
    }

    size_t                      num_aps = JCFW_WIFI_STA_SCAN_SIZE_MAX;
    jcfw_wifi_sta_scan_result_t aps[num_aps];
    memset(aps, 0, sizeof(jcfw_wifi_sta_scan_result_t) * num_aps);

    jcfw_cli_printf(cli, "Scanning for nearby access points...\n\n");

    jcfw_result_e err = jcfw_wifi_sta_scan(aps, &num_aps);
    if (err != JCFW_RESULT_OK)
    {
        jcfw_cli_printf(cli, "error: Scan failed, %d", err);
        return EXIT_FAILURE;
    }

    // TODO(Caleb): JCFW tabluated print function

    size_t max_ssid_len = 0;
    for (size_t i = 0; i < num_aps; i++)
    {
        max_ssid_len = JCFW_MAX(max_ssid_len, strlen((char *)aps[i].ssid));
    }

    jcfw_cli_printf(
        cli,
        "%-*s %-4s %-15s\n",
        max_ssid_len,
        "SSID",
        "RSSI (dBm)",
        "CHANNEL");

    for (size_t i = 0; i < num_aps; i++)
    {
        jcfw_cli_printf(
            cli, "%-*.33s %10d %7d\n", max_ssid_len, aps[i].ssid, aps[i].rssi_dBm, aps[i].channel);
    }

    return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------------------------------

bool cli_init(void)
{
    jcfw_result_e err = jcfw_cli_init(&s_cli, "home-cli $ ", util_putchar, NULL, true);
    JCFW_ERROR_IF_FALSE(err == JCFW_RESULT_OK, false, "Unable to initialize the CLI");

    return true;
}

void cli_run(void *arg)
{
    jcfw_cli_print_prompt(&s_cli);

    while (1)
    {
        char c = getchar(); // NOTE(Caleb): Nonblocking, returns 0xFF if no input is received

        if (c != (char)EOF && jcfw_cli_process_char(&s_cli, c))
        {
            int exit_status = -1;

            jcfw_cli_dispatch_result_e result =
                jcfw_cli_dispatch(&s_cli, s_cmds, JCFW_ARRAYSIZE(s_cmds), &exit_status);

            switch (result)
            {
                case JCFW_CLI_CMD_DISPATCH_RESULT_NO_CMD:
                    jcfw_cli_printf(&s_cli, "\n");
                    break;

                case JCFW_CLI_CMD_DISPATCH_RESULT_CMD_NOT_FOUND:
                    jcfw_cli_printf(&s_cli, "\n> ERROR, Command not found\n\n");
                    break;

                case JCFW_CLI_CMD_DISPATCH_RESULT_INVALID_ARGS:
                    jcfw_cli_printf(&s_cli, "\n> ERROR, Implementation error\n\n");
                    break;

                case JCFW_CLI_CMD_DISPATCH_RESULT_CLI_ERROR:
                    jcfw_cli_printf(&s_cli, "\n> ERROR, Internal CLI error\n\n");
                    break;

                case JCFW_CLI_CMD_DISPATCH_RESULT_OK:
                    if (exit_status == EXIT_SUCCESS)
                    {
                        jcfw_cli_printf(&s_cli, "\n> OK\n\n");
                    }
                    else
                    {
                        jcfw_cli_printf(&s_cli, "\n> ERROR, %d\n\n", exit_status);
                    }
            }

            jcfw_cli_print_prompt(&s_cli);
        }

        if (g_is_als_data_ready)
        {
            uint16_t      ch0         = 0x0000;
            uint16_t      ch1         = 0x0000;
            uint8_t       gain_factor = 0;
            jcfw_result_e err         = jcfw_ltr303_read(&g_ltr303, &ch0, &ch1, &gain_factor);
            if (err != JCFW_RESULT_OK)
            {
                jcfw_cli_printf(&s_cli, "error: Unable to read ALS data\n");
            }
            else if (gain_factor == 0)
            {
                jcfw_cli_printf(&s_cli, "error: Invalid gain read\n");
            }
            else
            {
                float visible_light = JCFW_CLAMP((float)(ch0 - ch1), 0, 64000) / gain_factor;

                jcfw_cli_printf(&s_cli, "ALS DATA: %f lux\n", visible_light);
            }

            g_is_als_data_ready = false;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
