#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

/*
 * Get and set the exit status of the most recently executed command ($?).
 */
int get_last_status(void);
void set_last_status(int status);

/*
 * Execute a single command (built-in or external).
 * If should_exit is not NULL and an exit command was run, sets *should_exit = 1.
 * Returns the command exit status.
 */
int execute_command(command_t *cmd, int *should_exit);

/*
 * Execute an entire pipeline of commands (M3 scope).
 * If should_exit is not NULL and an exit command was run, sets *should_exit = 1.
 * Returns the exit status of the pipeline.
 */
int execute_pipeline(pipeline_t *pipeline, int *should_exit);

#endif
