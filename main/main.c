#include "driver/uart.h"

#include "freertos/FreeRTOS.h"

#include "jcfw/cli.h"
#include "jcfw/driver/als/ltr303.h"
#include "jcfw/trace.h"
#include "jcfw/util/assert.h"
#include "jcfw/util/math.h"

#include "platform.h"
#include "util.h"

// -------------------------------------------------------------------------------------------------

static jcfw_cli_t s_cli = {0};

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

const jcfw_cli_cmd_spec_t s_cmds[] = {
    {
        .name        = "als",
        .usage       = "usage: als <on|off>",
        .handler     = als,
        .num_subcmds = 0,
        .subcmds     = NULL,
    },
};

// -------------------------------------------------------------------------------------------------

void app_main(void)
{
    // uart_event_t uart_event;

    JCFW_ASSERT(
        jcfw_platform_init() == JCFW_RESULT_OK, "error: Unable to execute platform initialization");

    jcfw_trace_init(util_putchar, NULL);
    jcfw_trace_set_level(JCFW_TRACE_LEVEL_WARN);

    JCFW_TRACELN_ERROR("MAIN", "Here's an error message!");
    JCFW_TRACELN_WARN("MAIN", "Here's an warning message!");
    JCFW_TRACELN("MAIN", "Here's an info message!");
    JCFW_TRACELN_DEBUG("MAIN", "Here's a debug message!");
    JCFW_TRACELN_NOTIFICATION("MAIN", "Here's a notification message!");

    jcfw_cli_init(&s_cli, "home-cli $ ", util_putchar, NULL);
    // JCFW_TRACELN_NOTIFICATION("MAIN", "CLI initialized!");
    // JCFW_TRACEHEX_NOTIFICATION("MAIN", &s_cli, sizeof(s_cli), "CLI Contents");

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
                    break;

                case JCFW_CLI_CMD_DISPATCH_RESULT_CMD_NOT_FOUND:
                    jcfw_cli_printf(&s_cli, "\n> ERROR, Command not found\n\n");
                    break;

                case JCFW_CLI_CMD_DISPATCH_RESULT_INVALID_ARG:
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
            uint16_t      ch0 = 0x0000;
            uint16_t      ch1 = 0x0000;
            jcfw_result_e err = jcfw_ltr303_read(&g_ltr303, &ch0, &ch1);
            JCFW_ASSERT(err == JCFW_RESULT_OK, "Unable to read ALS data");

            JCFW_TRACE("MAIN", "ALS DATA: %d lux\n", JCFW_CLAMP(ch0 - ch1, 0x0000, 0xFFFF));
            g_is_als_data_ready = false;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
