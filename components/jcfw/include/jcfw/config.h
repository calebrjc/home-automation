#ifndef __JCFW_CONFIG_H__
#define __JCFW_CONFIG_H__

// CLI ---------------------------------------------------------------------------------------------

/// @brief The maximum length for one command.
#define JCFW_CLI_MAX_LINE_LEN          120

/// @brief The size of the history buffer in bytes.
#define JCFW_CLI_HISTORY_BUFFER_SIZE   1024

/// @brief The maximum number of tokens allowed within a command.
#define JCFW_CLI_ARGC_MAX              16

/// @brief The maximum length of the prompt string.
#define JCFW_CLI_PROMPT_LEN_MAX        16

/// @brief Translate '\\r' to '\\n' for input and output "\\r\\n".
#define JCFW_CLI_SERIAL_TERM_TRANSLATE 1

// TRACE -------------------------------------------------------------------------------------------

#define JCFW_TRACE_MAX_TAG_LEN         6

#endif // __JCFW_CONFIG_H__
