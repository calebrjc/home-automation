#ifndef __JCFW_CLI_H__
#define __JCFW_CLI_H__

#include "jcfw/detail/common.h"
#include "jcfw/util/result.h"

#define JCFW_CLI_HISTORY_ENABLED (JCFW_CLI_HISTORY_BUFFER_SIZE > 0)

typedef struct jcfw_cli_s          jcfw_cli_t;
typedef struct jcfw_cli_cmd_spec_s jcfw_cli_cmd_spec_t;

/// @brief A function which the CLI uses for output.
/// @param param A parameter which has been passed through from the CLI. See jcfw_cli_init().
/// @param c The character to be output.
/// @param flush True if the "buffer" should be flushed. Useful for implementing buffering.
typedef void (*jcfw_cli_putc_f)(void *param, char c, bool flush);

/// @brief A function used to handle a CLI command.
/// @param cli The CLI for which this command is being handled.
/// @param argc The number of arguments passed to the handler (includes the command name).
/// @param argv The arguments passed to the handler.
typedef int (*jcfw_cli_handler_f)(jcfw_cli_t *cli, int argc, char **argv);

/// @brief The result of a CLI command dispatch operation.
typedef enum
{
    /// @brief The command was found and executed, or the help text was shown.
    JCFW_CLI_CMD_DISPATCH_RESULT_OK = 0,

    /// @brief The CLI API was provided with invalid arguments (this is an application error, not a
    /// user error)
    JCFW_CLI_CMD_DISPATCH_RESULT_INVALID_ARGS,

    /// @brief The CLI API has encountered an error internally.
    JCFW_CLI_CMD_DISPATCH_RESULT_CLI_ERROR,

    /// @brief No command was found which corresponded to the command buffer.
    JCFW_CLI_CMD_DISPATCH_RESULT_CMD_NOT_FOUND,

    /// @brief The input was empty.
    JCFW_CLI_CMD_DISPATCH_RESULT_NO_CMD,
} jcfw_cli_dispatch_result_e;

/// @brief This is the structure which represents the context of a CLI. It should not be accessed
/// directly by application code.
struct jcfw_cli_s
{
    char    prompt[JCFW_CLI_PROMPT_LEN_MAX];
    uint8_t flags;

    char   buffer[JCFW_CLI_MAX_LINE_LEN];
    size_t buffer_ptr;
    size_t cursor_pos;
    char  *argv[JCFW_CLI_ARGC_MAX];

    jcfw_cli_putc_f putc;
    void           *putc_param;

    size_t csi_counter;

    bool echo;

#if JCFW_CLI_HISTORY_ENABLED
    char history_buffer[JCFW_CLI_HISTORY_BUFFER_SIZE];
    int  history_idx;
    bool searching;
#endif
};

/// @brief The specification of a CLI command.
struct jcfw_cli_cmd_spec_s
{
    /// @brief The name that the command should be accessible by.
    const char *name;

    /// @brief The usage message for this command (for instance, as displayed by `--help`).
    const char *usage;

    /// @brief The handler function for this command.
    jcfw_cli_handler_f handler;

    /// @brief Subcommands contained within this top level command.
    jcfw_cli_cmd_spec_t *subcmds;

    /// @brief The number of subcommands within this top level command.
    size_t num_subcmds;
};

/// @brief Initialize a CLI. This function should only be called once.
/// @param cli The CLI structure to initialize.
/// @param prompt Optional; The prompt to use for the CLI. Must be null-terminated.
/// @param putc_func Required; The function to use for output.
/// @param putc_param Optional; A parameter to pass to the output function.
/// @param echo True if the terminal should echo input, and false otherwise.
/// @return JCFW_RESULT_OK if initalization is successful, or an error code otherwise.
jcfw_result_e jcfw_cli_init(
    jcfw_cli_t *cli, const char *prompt, jcfw_cli_putc_f putc_func, void *putc_param, bool echo);

/// @brief Process a new character for the given CLI. This function should not be called from an
/// interrupt handler.
/// @param cli The CLI to process the character for.
/// @param c The character to process.
/// @return True if the buffer should be processed, false otherwise.
bool jcfw_cli_process_char(jcfw_cli_t *cli, char c);

/// @brief Return the null-terminated command buffer, or NULL if the buffer is not ready to be
/// processed.
/// @param cli The CLI to get the command buffer from.
/// @return The null-terminated command buffer, or NULL if the buffer is not ready to be processed.
const char *jcfw_cli_getline(jcfw_cli_t *cli);

/// @brief Parse the command buffer into an argc/argv pair.
/// @param cli The CLI to process the command buffer of.
/// @param o_argv A pointer to be set to the argv array.
/// @return Argc on success, -1 on failure or if the command buffer is not ready to be processed.
int jcfw_cli_parse_args(jcfw_cli_t *cli, char ***o_argv);

/// @brief Find the command being invoked within `cmds`, and execute if if possible.
/// @param cli The CLI with which to execute the command.
/// @param cmds The commands to search for the invocation in.
/// @param num_cmds The number of top level commands provided.
/// @return The result of the command, or -1 if an error occurs (most likely "command not found").

/// @brief Find the command corresponding to the command buffer and execute it. If "--help" is found
/// in the command buffer, then the commands help text will be output and no execution occurs.
/// @param cli The CLI to execute the command with.
/// @param cmds The commands to search for the invocation in.
/// @param num_cmds The number of top level commands provided.
/// @param o_exit_status Required; The result of the command, if executed.
/// @return A member of `jcfw_cli_dispatch_result_e` (see the enum for details).
jcfw_cli_dispatch_result_e jcfw_cli_dispatch(
    jcfw_cli_t *cli, const jcfw_cli_cmd_spec_t *cmds, size_t num_cmds, int *o_exit_status);

/// @brief Output the prompt using the `putc_func` used to initialize the CLI. This function should
/// be called after the CLI has been initialized and is ready to receive commands, and after the
/// command buffer has been processed.
/// @param cli The CLI for which to output the prompt.
void jcfw_cli_print_prompt(const jcfw_cli_t *cli);

/// @brief Print a formatted string to the CLI output.
/// @param cli The CLI to output to.
/// @param format The format of the output. (see: printf)
/// @param ... The arguments used to populate the format.
void jcfw_cli_printf(const jcfw_cli_t *cli, const char *format, ...);

#endif // __JCFW_CLI_H__
