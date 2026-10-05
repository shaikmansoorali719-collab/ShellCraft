#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"

/*
 * Check whether a command is a built-in command.
 * Returns 1 if built-in, 0 if external command.
 */
int is_builtin(const command_t *cmd);

/*
 * Execute a built-in command.
 *
 * Parameters:
 *   cmd         - pointer to command structure
 *   should_exit - set to 1 if the shell should terminate (e.g. exit command)
 *
 * Returns:
 *   Exit status of the built-in (0 for success, non-zero for failure).
 */
int execute_builtin(command_t *cmd, int *should_exit);

#endif
