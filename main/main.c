#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "jcfw/cli.h"
#include "jcfw/util/assert.h"
#include "jcfw/util/math.h"

static jcfw_cli_t s_cli = {0};

// -------------------------------------------------------------------------------------------------

static int test(jcfw_cli_t *cli, int argc, char **argv)
{
    jcfw_cli_printf(cli, "It works!\n");

    jcfw_cli_printf(cli, "\n%d args:\n", argc);
    for (int i = 0; i < argc; i++)
    {
        jcfw_cli_printf(cli, "Arg %d/%d: '%s'\n", i + 1, argc, argv[i]);
    }

    if (argc > 1 && strcmp(argv[1], "bad") == 0)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int subtest(jcfw_cli_t *cli, int argc, char **argv)
{
    jcfw_cli_printf(cli, "Subcommand!\n");

    return EXIT_SUCCESS;
}

static int subsubtest(jcfw_cli_t *cli, int argc, char **argv)
{
    jcfw_cli_printf(cli, "Subsubcommand!\n");

    return EXIT_SUCCESS;
}

static int test2(jcfw_cli_t *cli, int argc, char **argv)
{
    jcfw_cli_printf(cli, "It works2!\n");

    return EXIT_SUCCESS;
}

jcfw_cli_cmd_spec_t s_cmds[] = {
    {
        .name        = "test",
        .usage       = "usage: test [ARGS...]",
        .handler     = test,
        .num_subcmds = 1,
        .subcmds =
            (jcfw_cli_cmd_spec_t[]) {
                {
                    .name        = "subtest",
                    .usage       = "usage: subtest [ARGS...]",
                    .handler     = subtest,
                    .num_subcmds = 1,
                    .subcmds =
                        (jcfw_cli_cmd_spec_t[]) {
                            {
                                .name        = "subsubtest",
                                .usage       = "usage: subsubtest [ARGS...]",
                                .handler     = subsubtest,
                                .num_subcmds = 0,
                                .subcmds     = NULL,
                            },
                        },
                },
            },
    },
    {
        .name        = "test2",
        .usage       = "usage: test2 [ARGS...]",
        .handler     = test2,
        .num_subcmds = 0,
        .subcmds     = NULL,
    },
};

// -------------------------------------------------------------------------------------------------

static void app_putchar(void *data, char ch, bool flush);

void app_main(void)
{
    jcfw_cli_init(&s_cli, "home-cli $ ", app_putchar, stdout);
    jcfw_cli_print_prompt(&s_cli);

    while (1)
    {
        char c = getchar();

        if (c != (char)EOF && jcfw_cli_process_char(&s_cli, c))
        {
            int exit_status = -1;

            jcfw_cli_cmd_dispatch_result_e result =
                jcfw_cli_dispatch_command(&s_cli, s_cmds, JCFW_ARRAYSIZE(s_cmds), &exit_status);

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

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

static void app_putchar(void *data, char c, bool flush)
{
    JCFW_ASSERT_RET(data);

    FILE *f = data;

    fputc(c, f);

    if (flush)
    {
        fflush(f);
    }
}
