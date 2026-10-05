#ifndef HELP_H
#define HELP_H

/* Structure representing a built-in command and its help documentation */
typedef struct {
    const char *name;        /* Command name */
    const char *summary;     /* Short one-line summary for general help */
    const char *description; /* Detailed description for command-specific help */
    const char *usage;       /* Usage syntax */
} builtin_cmd_t;

/*
 * Display general help or command-specific help.
 * If cmd is NULL or empty, prints all available built-in commands.
 * If cmd is provided, prints detailed help for that command,
 * or an informative error message if the command is unrecognized.
 */
void display_help(const char *cmd);

/*
 * Parse and handle a 'help' command line if matched.
 * Returns 1 if the command was a help command and was handled,
 * or 0 if the command was not a help command.
 */
int handle_help(const char *cmd_line);

#endif
