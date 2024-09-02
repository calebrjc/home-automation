#include "jcfw/cli.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "jcfw/util/assert.h"
#include "jcfw/util/bit.h"

// -------------------------------------------------------------------------------------------------

#define JCFW_CLI_ANSI_MOVE_TO_BOL  "\x1b[1G"
#define JCFW_CLI_ANSI_CLEAR_TO_EOL "\x1b[0K"

// -------------------------------------------------------------------------------------------------

typedef enum
{
    JCFW_CLI_FLAGS_NONE      = 0,
    JCFW_CLI_FLAGS_CMD_READY = JCFW_BIT(0),
    JCFW_CLI_FLAGS_ESC       = JCFW_BIT(1),
    JCFW_CLI_FLAGS_CSI       = JCFW_BIT(2),
} jcfw_cli_flags_e;

// -------------------------------------------------------------------------------------------------

static bool _jcfw_cli_is_whitespace(char c);
static void _jcfw_cli_reset(jcfw_cli_t *cli);
static void _jcfw_cli_handle_char_default(jcfw_cli_t *cli, char c);

static void _jcfw_cli_term_ansi(jcfw_cli_t *cli, size_t n, char code);
static void _jcfw_cli_term_cursor_back(jcfw_cli_t *cli, size_t n);
static void _jcfw_cli_term_cursor_fwd(jcfw_cli_t *cli, size_t n);

#if JCFW_CLI_HISTORY_ENABLED
static void _jcfw_cli_term_backspace(jcfw_cli_t *cli, size_t n);
#endif

#if JCFW_CLI_HISTORY_ENABLED
static void        _jcfw_cli_history_append(jcfw_cli_t *cli);
static const char *_jcfw_cli_history_get(jcfw_cli_t *cli, ssize_t history_idx);
static const char *_jcfw_cli_history_find_substring(jcfw_cli_t *cli, const char *substr);
#endif

#if JCFW_CLI_HISTORY_ENABLED
static void _jcfw_cli_search_mode_start(jcfw_cli_t *cli);
static void _jcfw_cli_search_mode_stop(jcfw_cli_t *cli, bool print);
#endif

static void _jcfw_cli_putc(const jcfw_cli_t *cli, char c, bool flush);
static void _jcfw_cli_puts(const jcfw_cli_t *cli, const char *s);

#define _jcfw_cli_internal_putc(_cli, _c, _flush)                                                  \
    do                                                                                             \
    {                                                                                              \
        if (_cli->echo)                                                                            \
        {                                                                                          \
            _jcfw_cli_putc(_cli, _c, _flush);                                                      \
        }                                                                                          \
    } while (0)

#define _jcfw_cli_internal_puts(_cli, _s)                                                          \
    do                                                                                             \
    {                                                                                              \
        if (_cli->echo)                                                                            \
        {                                                                                          \
            _jcfw_cli_puts(_cli, _s);                                                              \
        }                                                                                          \
    } while (0)

#define _jcfw_cli_internal_printf(_cli, ...)                                                       \
    do                                                                                             \
    {                                                                                              \
        if (_cli->echo)                                                                            \
        {                                                                                          \
            jcfw_cli_printf(_cli, ##__VA_ARGS__);                                                  \
        }                                                                                          \
    } while (0)

static const jcfw_cli_cmd_spec_t *_jcfw_cli_find_cmd(
    const jcfw_cli_cmd_spec_t *cmds, size_t num_cmds, int argc, char **argv, size_t *o_depth);

// -------------------------------------------------------------------------------------------------

jcfw_result_e jcfw_cli_init(
    jcfw_cli_t *cli, const char *prompt, jcfw_cli_putc_f putc_func, void *putc_param, bool echo)
{
    JCFW_ERROR_IF_FALSE(cli, JCFW_RESULT_INVALID_ARGS, "No CLI provided");
    JCFW_ERROR_IF_FALSE(putc_func, JCFW_RESULT_INVALID_ARGS, "No output function provided");

    memset(cli, 0, sizeof(*cli));
    cli->putc       = putc_func;
    cli->putc_param = putc_param;

    if (prompt)
    {
        strncpy(cli->prompt, prompt, sizeof(cli->prompt));
        cli->prompt[sizeof(cli->prompt) - 1] = '\0';
    }

    cli->echo = echo;

    _jcfw_cli_reset(cli);

    return JCFW_RESULT_OK;
}

bool jcfw_cli_process_char(jcfw_cli_t *cli, char c)
{
    JCFW_ERROR_IF_FALSE(cli, JCFW_RESULT_INVALID_ARGS, "No CLI provided");

    if (cli->flags & JCFW_CLI_FLAGS_CMD_READY)
    {
        _jcfw_cli_reset(cli);
    }

    if (cli->flags & JCFW_CLI_FLAGS_CSI)
    {
        if ('0' <= c && c <= '9' && cli->csi_counter < 100)
        {
            cli->csi_counter = (cli->csi_counter * 10) + (c - '0');
        }
        else
        {
            if (cli->csi_counter == 0)
            {
                cli->csi_counter++;
            }

            switch (c)
            {
                case 'A': // UP
                {
#if JCFW_CLI_HISTORY_ENABLED
                    _jcfw_cli_term_backspace(cli, cli->cursor_pos);

                    const char *result = _jcfw_cli_history_get(cli, cli->history_idx + 1);
                    if (result)
                    {
                        cli->history_idx++;
                        strncpy(cli->buffer, result, sizeof(cli->buffer));
                        cli->buffer[sizeof(cli->buffer) - 1] = '\0';

                        size_t len      = strlen(result);
                        cli->buffer_ptr = len;
                        cli->cursor_pos = len;

                        _jcfw_cli_internal_printf(
                            cli, "%s%s", cli->buffer, JCFW_CLI_ANSI_CLEAR_TO_EOL);
                    }
                    else
                    {
                        int cached_history_idx = cli->history_idx;
                        _jcfw_cli_reset(cli);
                        cli->history_idx = cached_history_idx;

                        _jcfw_cli_internal_puts(cli, JCFW_CLI_ANSI_CLEAR_TO_EOL);
                    }
#endif
                    break;
                }

                case 'B': // DOWN
                {
#if JCFW_CLI_HISTORY_ENABLED
                    _jcfw_cli_term_backspace(cli, cli->cursor_pos);

                    const char *result = _jcfw_cli_history_get(cli, cli->history_idx - 1);
                    if (result)
                    {
                        cli->history_idx--;
                        strncpy(cli->buffer, result, sizeof(cli->buffer));
                        cli->buffer[sizeof(cli->buffer) - 1] = '\0';

                        int len         = strlen(result);
                        cli->buffer_ptr = len;
                        cli->cursor_pos = len;

                        _jcfw_cli_internal_printf(
                            cli, "%s%s", cli->buffer, JCFW_CLI_ANSI_CLEAR_TO_EOL);
                    }
                    else
                    {
                        // NOTE(Caleb): In-progress commands cannot be shown since history
                        // overwrites the working buffer. Ergo, we just clear the line.
                        _jcfw_cli_reset(cli);
                        _jcfw_cli_internal_puts(cli, JCFW_CLI_ANSI_CLEAR_TO_EOL);
                    }
#endif
                    break;
                }

                case 'C': // RIGHT
                    if (cli->cursor_pos <= cli->buffer_ptr - cli->csi_counter)
                    {
                        cli->cursor_pos += cli->csi_counter;
                        _jcfw_cli_term_cursor_fwd(cli, cli->csi_counter);
                    }

                    break;

                case 'D': // LEFT
                    if (cli->cursor_pos >= cli->csi_counter)
                    {
                        cli->cursor_pos -= cli->csi_counter;
                        _jcfw_cli_term_cursor_back(cli, cli->csi_counter);
                    }

                    break;

                case 'F': // END
                    _jcfw_cli_term_cursor_fwd(cli, cli->buffer_ptr - cli->cursor_pos);
                    cli->cursor_pos = cli->buffer_ptr;

                    break;

                case 'H': // HOME
                    _jcfw_cli_term_cursor_back(cli, cli->cursor_pos);
                    cli->cursor_pos = 0;

                    break;

                case '~': // DEL
                    if (cli->csi_counter == 3 && cli->cursor_pos < cli->buffer_ptr)
                    {
                        memmove(
                            &cli->buffer[cli->cursor_pos],
                            &cli->buffer[cli->cursor_pos + 1],
                            cli->buffer_ptr - cli->cursor_pos);
                        cli->buffer_ptr--;

                        _jcfw_cli_internal_printf(cli, "%s ", &cli->buffer[cli->cursor_pos]);
                        _jcfw_cli_term_cursor_back(cli, cli->buffer_ptr - cli->cursor_pos + 1);
                    }

                    break;

                default:
                    // TODO(Caleb): Handle more escape sequences?
                    break;
            }

            cli->flags       = 0;
            cli->csi_counter = 0;
        }
    }
    else
    {
        switch (c)
        {
            case '\0':
                break;

            case '\x01': // Ctrl-A - HOME
                _jcfw_cli_term_cursor_back(cli, cli->cursor_pos);
                cli->cursor_pos = 0;

                break;

            case '\x05': // Ctrl-E - End
                _jcfw_cli_term_cursor_fwd(cli, cli->buffer_ptr - cli->cursor_pos);
                cli->cursor_pos = cli->buffer_ptr;

                break;

            case '\x03': // Ctrl-C - Cancel current input
                _jcfw_cli_reset(cli);
                _jcfw_cli_internal_printf(cli, "^C\n%s", cli->prompt);

                break;

            case '\x0b': // Ctrl-K
                cli->buffer[cli->cursor_pos] = '\0';
                cli->buffer_ptr              = cli->cursor_pos;

                _jcfw_cli_internal_puts(cli, JCFW_CLI_ANSI_CLEAR_TO_EOL);

                break;

            case '\x0c': // Ctrl-L
                _jcfw_cli_internal_printf(
                    cli,
                    "%s%s%s%s",
                    JCFW_CLI_ANSI_MOVE_TO_BOL,
                    JCFW_CLI_ANSI_CLEAR_TO_EOL,
                    cli->prompt,
                    cli->buffer);

                _jcfw_cli_term_cursor_back(cli, cli->buffer_ptr - cli->cursor_pos);

                break;

            case '\b':   // BACKSPACE
            case '\x7f': // Also BACKSPACE?
#if JCFW_CLI_HISTORY_ENABLED
                if (cli->searching)
                {
                    _jcfw_cli_search_mode_stop(cli, true);
                }
#endif
                if (cli->cursor_pos > 0)
                {
                    memmove(
                        &cli->buffer[cli->cursor_pos - 1],
                        &cli->buffer[cli->cursor_pos],
                        cli->buffer_ptr - cli->cursor_pos + 1);
                    cli->cursor_pos--;
                    cli->buffer_ptr--;

                    _jcfw_cli_term_cursor_back(cli, 1);
                    _jcfw_cli_internal_printf(cli, "%s ", &cli->buffer[cli->cursor_pos]);
                    _jcfw_cli_term_cursor_back(cli, cli->buffer_ptr - cli->cursor_pos + 1);
                }

                break;

            case '\x12': // Ctrl-R
#if JCFW_CLI_HISTORY_ENABLED
                if (!cli->searching)
                {
                    _jcfw_cli_search_mode_start(cli);
                }
#endif
                break;

            case '\x1b': // ESC sequence start
#if JCFW_CLI_HISTORY_ENABLED
                if (cli->searching)
                {
                    _jcfw_cli_search_mode_stop(cli, true);
                }
#endif
                JCFW_BITCLEAR(cli->flags, JCFW_CLI_FLAGS_CSI);
                JCFW_BITSET(cli->flags, JCFW_CLI_FLAGS_ESC);
                cli->csi_counter = 0;

                break;

            case '[': // CSI start
                if (cli->flags & JCFW_CLI_FLAGS_ESC)
                {
                    JCFW_BITSET(cli->flags, JCFW_CLI_FLAGS_CSI);
                }
                else
                {
                    _jcfw_cli_handle_char_default(cli, c);
                }

                break;

#if JCFW_CLI_SERIAL_TERM_TRANSLATE
            case '\r':
                c = '\n';

                // NOTE(Caleb): Intentional fallthrough
                __attribute__((fallthrough));
#endif
            case '\n':
                _jcfw_cli_putc(cli, '\n', true);
                break;

            default:
                _jcfw_cli_handle_char_default(cli, c);
        }
    }

    if (c == '\n')
    {
        JCFW_BITSET(cli->flags, JCFW_CLI_FLAGS_CMD_READY);
    }

    if (cli->flags & JCFW_CLI_FLAGS_CMD_READY)
    {
#if JCFW_CLI_HISTORY_ENABLED
        if (cli->searching)
        {
            _jcfw_cli_search_mode_stop(cli, false);
        }

        _jcfw_cli_history_append(cli);
#endif
    }
    return cli->flags & JCFW_CLI_FLAGS_CMD_READY;
}

const char *jcfw_cli_getline(jcfw_cli_t *cli)
{
    JCFW_ERROR_IF_FALSE(cli, NULL, "No CLI provided");
    JCFW_RETURN_IF_FALSE(cli->flags & JCFW_CLI_FLAGS_CMD_READY, NULL);

    return cli->buffer;
}

int jcfw_cli_parse_args(jcfw_cli_t *cli, char ***argv)
{
    JCFW_ERROR_IF_FALSE(cli, -1, "No CLI provided");
    JCFW_ERROR_IF_FALSE(argv, -1, "No storage provided for arguments");
    JCFW_RETURN_IF_FALSE(cli->flags & JCFW_CLI_FLAGS_CMD_READY, -1);

    int  pos       = 0;
    bool in_arg    = false;
    bool in_escape = false;
    char in_string = '\0';

    for (size_t i = 0; i < sizeof(cli->buffer) && cli->buffer[i] != '\0'; i++)
    {
        if (in_escape)
        {
            in_escape = false;
            continue;
        }

        if (in_string)
        {
            if (cli->buffer[i] == in_string)
            {
                memmove(&cli->buffer[i], &cli->buffer[i + 1], sizeof(cli->buffer) - i - 1);
                in_string = '\0';
                i--;
            }

            continue;
        }

        if (_jcfw_cli_is_whitespace(cli->buffer[i]))
        {
            if (in_arg)
            {
                cli->buffer[i] = '\0';
            }

            in_arg = false;
            continue;
        }

        if (!in_arg)
        {
            if (pos >= JCFW_CLI_ARGC_MAX)
            {
                break;
            }
            cli->argv[pos] = &cli->buffer[i];
            pos++;
            in_arg = true;
        }

        if (cli->buffer[i] == '\\')
        {
            memmove(&cli->buffer[i], &cli->buffer[i + 1], sizeof(cli->buffer) - i - 1);
            i--;
            in_escape = true;
        }

        if (cli->buffer[i] == '\'' || cli->buffer[i] == '"')
        {
            in_string = cli->buffer[i];
            memmove(&cli->buffer[i], &cli->buffer[i + 1], sizeof(cli->buffer) - i - 1);
            i--;
        }
    }

    // NOTE(Caleb): Traditionally, argv[argc] == NULL
    if (pos >= JCFW_CLI_ARGC_MAX)
    {
        pos--;
    }
    cli->argv[pos] = NULL;

    *argv = cli->argv;

    return pos;
}

void jcfw_cli_print_prompt(const jcfw_cli_t *cli)
{
    JCFW_ERROR_IF_FALSE(cli, , "No CLI provided");
    JCFW_RETURN_IF_FALSE(cli->prompt);

    _jcfw_cli_puts(cli, cli->prompt);
}

void jcfw_cli_printf(const jcfw_cli_t *cli, const char *format, ...)
{
    JCFW_ERROR_IF_FALSE(cli, , "No CLI provided");
    JCFW_ERROR_IF_FALSE(format, , "No format provided");

    va_list args;
    va_start(args, format);

    size_t size = (size_t)vsnprintf(NULL, 0, format, args) + 1;
    char   buffer[size];
    vsnprintf(buffer, size, format, args);

    va_end(args);

    buffer[size - 1] = '\0';
    _jcfw_cli_puts(cli, buffer);
}

jcfw_cli_dispatch_result_e jcfw_cli_dispatch(
    jcfw_cli_t *cli, const jcfw_cli_cmd_spec_t *cmds, size_t num_cmds, int *o_exit_status)
{
    JCFW_ERROR_IF_FALSE(cli, JCFW_CLI_CMD_DISPATCH_RESULT_INVALID_ARGS, "No CLI provided");
    JCFW_ERROR_IF_FALSE(
        cmds && num_cmds, JCFW_CLI_CMD_DISPATCH_RESULT_INVALID_ARGS, "No commands provided");
    JCFW_ERROR_IF_FALSE(
        o_exit_status,
        JCFW_CLI_CMD_DISPATCH_RESULT_INVALID_ARGS,
        "No storage for the exit status provided");

    char **argv = NULL;
    int    argc = jcfw_cli_parse_args(cli, &argv);
    if (argc < 0)
    {
        return JCFW_CLI_CMD_DISPATCH_RESULT_CLI_ERROR;
    }
    else if (argc == 0)
    {
        return JCFW_CLI_CMD_DISPATCH_RESULT_NO_CMD;
    }

    // JCFW_TRACELN_DEBUG("JCFW-CLI", "argc = %d", argc);
    // JCFW_TRACELN_DEBUG("JCFW-CLI", "argv:");
    // for (int i = 0; i < argc; i++)
    // {
    //     JCFW_TRACELN_DEBUG("JCFW-CLI", "[%d] = %s", i, argv[i]);
    // }

    size_t                     cmd_depth = 0;
    const jcfw_cli_cmd_spec_t *cmd = _jcfw_cli_find_cmd(cmds, num_cmds, argc, argv, &cmd_depth);
    JCFW_RETURN_IF_FALSE(cmd && cmd->handler, JCFW_CLI_CMD_DISPATCH_RESULT_CMD_NOT_FOUND);

    for (size_t i = cmd_depth; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            if (cmd->usage)
            {
                jcfw_cli_printf(cli, "%s\n", cmd->usage);
            }
            else
            {
                jcfw_cli_printf(cli, "No help found for the specified command\n");
            }

            *o_exit_status = EXIT_SUCCESS;
            return JCFW_CLI_CMD_DISPATCH_RESULT_OK;
        }
    }

    // JCFW_TRACELN_DEBUG("JCFW-CLI", "cmd_depth = %zu", cmd_depth);
    // JCFW_TRACELN_DEBUG("JCFW-CLI", "passed argc = %d", argc - cmd_depth);
    // JCFW_TRACELN_DEBUG("JCFW-CLI", "passed argv:");
    // for (int i = 0; i < argc - cmd_depth; i++)
    // {
    //     JCFW_TRACELN_DEBUG("JCFW-CLI", "[%d] = %s", i, (argv + cmd_depth)[i]);
    // }

    *o_exit_status = cmd->handler(cli, argc - cmd_depth, argv + cmd_depth);
    return JCFW_CLI_CMD_DISPATCH_RESULT_OK;
}

// -------------------------------------------------------------------------------------------------

static bool _jcfw_cli_is_whitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static void _jcfw_cli_reset(jcfw_cli_t *cli)
{
    JCFW_RETURN_IF_FALSE(cli);

    cli->buffer[0]   = '\0';
    cli->buffer_ptr  = 0;
    cli->cursor_pos  = 0;
    cli->csi_counter = 0;
    cli->flags       = 0;

#if JCFW_CLI_HISTORY_ENABLED
    cli->history_idx = -1;
    cli->searching   = false;
#endif
}

static void _jcfw_cli_handle_char_default(jcfw_cli_t *cli, char c)
{
    JCFW_RETURN_IF_FALSE(cli->buffer_ptr < sizeof(cli->buffer) - 1);

    memmove(
        &cli->buffer[cli->cursor_pos + 1],
        &cli->buffer[cli->cursor_pos],
        cli->buffer_ptr - cli->cursor_pos);

    cli->buffer[cli->cursor_pos++] = c;
    cli->buffer[++cli->buffer_ptr] = '\0';

#if JCFW_CLI_HISTORY_ENABLED
    if (cli->searching)
    {
        _jcfw_cli_puts(cli, JCFW_CLI_ANSI_MOVE_TO_BOL JCFW_CLI_ANSI_CLEAR_TO_EOL "search: ");

        const char *result = _jcfw_cli_history_find_substring(cli, cli->buffer);
        if (result)
        {
            _jcfw_cli_puts(cli, result);
        }
    }
    else
#endif
    {
        _jcfw_cli_internal_puts(cli, &cli->buffer[cli->cursor_pos - 1]);
        _jcfw_cli_term_cursor_back(cli, cli->buffer_ptr - cli->cursor_pos);
    }
}

static void _jcfw_cli_term_ansi(jcfw_cli_t *cli, size_t n, char code)
{
    JCFW_RETURN_IF_FALSE(cli);

    char buffer[5] = {'\x1b', '[', '0' + (n % 10), code, '\0'};
    _jcfw_cli_internal_puts(cli, buffer);
}

static void _jcfw_cli_term_cursor_back(jcfw_cli_t *cli, size_t n)
{
    JCFW_RETURN_IF_FALSE(cli && cli->echo);

    while (n > 0)
    {
        size_t count = n > 9 ? 9 : n;
        _jcfw_cli_term_ansi(cli, count, 'D');
        n -= count;
    }
}

static void _jcfw_cli_term_cursor_fwd(jcfw_cli_t *cli, size_t n)
{
    JCFW_RETURN_IF_FALSE(cli && cli->echo);

    while (n > 0)
    {
        size_t count = n > 9 ? 9 : n;
        _jcfw_cli_term_ansi(cli, count, 'C');
        n -= count;
    }
}

#if JCFW_CLI_HISTORY_ENABLED
static void _jcfw_cli_term_backspace(jcfw_cli_t *cli, size_t n)
{
    JCFW_RETURN_IF_FALSE(cli && cli->echo);

    while (n--)
    {
        _jcfw_cli_putc(cli, '\b', n == 0);
    }
}
#endif

#if JCFW_CLI_HISTORY_ENABLED
static void _jcfw_cli_history_append(jcfw_cli_t *cli)
{
    JCFW_RETURN_IF_FALSE(cli);

    if (cli->buffer_ptr <= 0 || strcmp(cli->buffer, cli->history_buffer) == 0)
    {
        return;
    }

    memmove(
        &cli->history_buffer[cli->buffer_ptr + 1],
        &cli->history_buffer[0],
        sizeof(cli->history_buffer) - cli->buffer_ptr + 1);
    memcpy(cli->history_buffer, cli->buffer, cli->buffer_ptr + 1);

    cli->history_buffer[sizeof(cli->history_buffer) - 1] = '\0';
}
#endif

#if JCFW_CLI_HISTORY_ENABLED
static const char *_jcfw_cli_history_get(jcfw_cli_t *cli, ssize_t history_idx)
{
    JCFW_RETURN_IF_FALSE(cli, NULL);

    if (history_idx < 0)
    {
        return NULL;
    }

    size_t pos = 0;
    for (int i = 0; i < history_idx; i++)
    {
        size_t len = strlen(&cli->history_buffer[pos]);
        if (len == 0)
        {
            return NULL;
        }

        pos += len + 1;
        if (pos == sizeof(cli->history_buffer))
        {
            return NULL;
        }
    }

    return &cli->history_buffer[pos];
}
#endif

#if JCFW_CLI_HISTORY_ENABLED
static const char *_jcfw_cli_history_find_substring(jcfw_cli_t *cli, const char *substr)
{
    JCFW_RETURN_IF_FALSE(cli, NULL);

    size_t i = 0;
    while (1)
    {
        const char *result = _jcfw_cli_history_get(cli, i);
        if (!result)
        {
            return NULL;
        }

        if (strstr(result, substr))
        {
            return result;
        }

        i++;
    }
}
#endif

#if JCFW_CLI_HISTORY_ENABLED
static void _jcfw_cli_search_mode_start(jcfw_cli_t *cli)
{
    JCFW_RETURN_IF_FALSE(cli);

    _jcfw_cli_puts(cli, "\nsearch: ");
    cli->searching = true;
}
#endif

#if JCFW_CLI_HISTORY_ENABLED
static void _jcfw_cli_search_mode_stop(jcfw_cli_t *cli, bool print)
{
    JCFW_RETURN_IF_FALSE(cli);

    const char *result = _jcfw_cli_history_find_substring(cli, cli->buffer);
    if (result)
    {
        strncpy(cli->buffer, result, sizeof(cli->buffer));
        cli->buffer[sizeof(cli->buffer) - 1] = '\0';
    }
    else
    {
        cli->buffer[0] = '\0';
    }

    cli->buffer_ptr = cli->cursor_pos = strlen(cli->buffer);
    cli->searching                    = false;

    if (print)
    {
        jcfw_cli_printf(
            cli,
            "%s%s%s%s",
            JCFW_CLI_ANSI_MOVE_TO_BOL,
            JCFW_CLI_ANSI_CLEAR_TO_EOL,
            cli->prompt,
            cli->buffer);
    }
}
#endif

static void _jcfw_cli_putc(const jcfw_cli_t *cli, char c, bool flush)
{
    JCFW_RETURN_IF_FALSE(cli && cli->putc);

#if JCFW_CLI_SERIAL_TERM_TRANSLATE
    if (c == '\n')
    {
        cli->putc(cli->putc_param, '\r', false);
    }
#endif
    cli->putc(cli->putc_param, c, flush);
}

static void _jcfw_cli_puts(const jcfw_cli_t *cli, const char *s)
{
    JCFW_RETURN_IF_FALSE(cli && s);

    while (*s)
    {
        _jcfw_cli_putc(cli, *s, s[1] == '\0');
        s++;
    }
}

static const jcfw_cli_cmd_spec_t *_jcfw_cli_find_cmd(
    const jcfw_cli_cmd_spec_t *cmds, size_t num_cmds, int argc, char **argv, size_t *o_depth)
{
    JCFW_RETURN_IF_FALSE(cmds && num_cmds && argc && argv && o_depth, NULL);

    const jcfw_cli_cmd_spec_t *current_cmds     = cmds;
    size_t                     current_num_cmds = num_cmds;
    const jcfw_cli_cmd_spec_t *last_found_cmd   = NULL;
    size_t                     depth            = 0;

    while (depth < argc)
    {
        const jcfw_cli_cmd_spec_t *found_cmd = NULL;

        for (size_t i = 0; i < current_num_cmds; i++)
        {
            if (strcmp(current_cmds[i].name, argv[depth]) == 0)
            {
                found_cmd      = &current_cmds[i];
                last_found_cmd = &current_cmds[i];
                break;
            }
        }

        if (found_cmd == NULL)
        {
            break;
        }

        *o_depth = depth;

        if (depth + 1 >= argc || !found_cmd->subcmds || found_cmd->num_subcmds <= 0)
        {
            break;
        }

        current_cmds     = found_cmd->subcmds;
        current_num_cmds = found_cmd->num_subcmds;
        depth++;
    }

    return last_found_cmd;
}
