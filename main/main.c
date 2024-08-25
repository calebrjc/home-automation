#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"

#include "jcfw/cli.h"
#include "jcfw/driver/als/ltr303.h"
#include "jcfw/platform/platform.h"
#include "jcfw/trace.h"
#include "jcfw/util/assert.h"
#include "jcfw/util/math.h"

#include "platform.h"

// -------------------------------------------------------------------------------------------------

static jcfw_cli_t s_cli = {0};

// -------------------------------------------------------------------------------------------------

void hw_init(void);

void i2c_mem_read(
    void          *arg,
    const uint8_t *mem_addr,
    size_t         mem_addr_size,
    uint8_t       *o_data,
    size_t         data_size,
    uint32_t       timeout_ms);

void i2c_mem_write(
    void          *arg,
    const uint8_t *mem_addr,
    size_t         mem_addr_size,
    const uint8_t *data,
    size_t         data_size,
    uint32_t       timeout_ms);

void on_als_data_ready(void *arg);

// -------------------------------------------------------------------------------------------------

static int als(jcfw_cli_t *cli, int argc, char **argv)
{
    if (argc != 2)
    {
        jcfw_cli_printf(cli, "usage: als <on|off>\n");
        return EXIT_FAILURE;
    }

    jcfw_result_e err = JCFW_RESULT_ERROR;

    if (strncmp(argv[1], "on", 2) == 0)
    {
        err = jcfw_ltr303_set_mode(&g_ltr303, JCFW_LTR303_MODE_ACTIVE);
        if (err != JCFW_RESULT_OK)
        {
            jcfw_cli_printf(cli, "error: Unable to put the ALS in active mode\n");
            return EXIT_FAILURE;
        }
    }
    else if (strncmp(argv[1], "off", 3) == 0)
    {
        err = jcfw_ltr303_set_mode(&g_ltr303, JCFW_LTR303_MODE_STANDBY);
        if (err != JCFW_RESULT_OK)
        {
            jcfw_cli_printf(cli, "error: Unable to put the ALS in standby mode\n");
            return EXIT_FAILURE;
        }
    }
    else
    {
        jcfw_cli_printf(cli, "usage: als <on|off>\n");
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

static void app_putchar(void *data, char ch, bool flush);

void app_main(void)
{
    if (jcfw_platform_init() != JCFW_RESULT_OK)
    {
        printf("ERROR!\n");
        return;
    }

    JCFW_TRACE("MAIN", "Here we go!\n");
    hw_init();

    jcfw_cli_init(&s_cli, "home-cli $ ", app_putchar, NULL);
    // JCFW_TRACEHEX("MAIN", &s_cli, sizeof(s_cli), "cli");
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

            printf("ALS DATA: %d lux\n", JCFW_CLAMP(ch0 - ch1, 0x0000, 0xFFFF));
            g_is_als_data_ready = false;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

static void app_putchar(void *data, char c, bool flush)
{
    (void)data;
    jcfw_platform_trace_putc(c, flush);
}

// -------------------------------------------------------------------------------------------------

void hw_init(void)
{
    // I2C Light Sensor --------------------------------------------------------

    jcfw_result_e err;

    uint16_t thresh_low  = 0x0000;
    uint16_t thresh_high = 0x0000;
    err                  = jcfw_ltr303_set_thresholds(&g_ltr303, &thresh_low, &thresh_high);
    JCFW_ASSERT(err == JCFW_RESULT_OK, "Unable to set thresholds");

    err = jcfw_ltr303_enable_interrupt(&g_ltr303, true);
    JCFW_ASSERT(err == JCFW_RESULT_OK, "Unable to enable interrupts");

    // err = jcfw_ltr303_set_mode(&g_ltr303, JCFW_LTR303_MODE_ACTIVE);
    // JCFW_ASSERT(err == JCFW_RESULT_OK, "Unable to enable ALS");
    // vTaskDelay(10 / portTICK_PERIOD_MS); // NOTE(Caleb): See datasheet
}
