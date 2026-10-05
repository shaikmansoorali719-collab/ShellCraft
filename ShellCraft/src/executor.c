#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "executor.h"
#include "builtin.h"

/*
 * Static state tracking the exit status of the previous command ($?).
 * Defaults to 0 upon shell startup.
 */
static int last_exit_status = 0;

int get_last_status(void)
{
    return last_exit_status;
}

void set_last_status(int status)
{
    last_exit_status = status;
}

/*
 * Execute a single command.
 * Dispatches to built-ins if recognized, otherwise creates a child process
 * via fork(), runs execvp(), and waits for completion via waitpid().
 */
int execute_command(command_t *cmd, int *should_exit)
{
    if (cmd == NULL || cmd->argc == 0 || cmd->argv[0] == NULL)
    {
        return 0;
    }

    /* 1. Built-in command handling */
    if (is_builtin(cmd))
    {
        int status = execute_builtin(cmd, should_exit);
        set_last_status(status);
        return status;
    }

    /* 2. External command execution */
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        set_last_status(1);
        return 1;
    }

    if (pid == 0)
    {
        /* CHILD PROCESS */
        cmd->argv[cmd->argc] = NULL;
        execvp(cmd->argv[0], cmd->argv);

        /* execvp returns only if an error occurred */
        if (errno == ENOENT)
        {
            fprintf(stderr, "shellcraft: %s: command not found\n", cmd->argv[0]);
            exit(127);
        }
        else if (errno == EACCES)
        {
            fprintf(stderr, "shellcraft: %s: Permission denied\n", cmd->argv[0]);
            exit(126);
        }
        else
        {
            perror("shellcraft");
            exit(EXIT_FAILURE);
        }
    }

    /* PARENT PROCESS */
    if (cmd->background)
    {
        /* Background job: display PID and return immediately */
        printf("[%d]\n", pid);
        set_last_status(0);
        return 0;
    }

    /* Foreground process: wait for completion and record exit status */
    int status = 0;
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        set_last_status(1);
        return 1;
    }

    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);
        set_last_status(exit_code);
        return exit_code;
    }
    else if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        int exit_code = 128 + sig;
        set_last_status(exit_code);
        return exit_code;
    }

    set_last_status(1);
    return 1;
}

/*
 * Execute a pipeline of commands (M3 sequential execution).
 */
int execute_pipeline(pipeline_t *pipeline, int *should_exit)
{
    if (pipeline == NULL || pipeline->command_count == 0)
    {
        return 0;
    }

    int status = 0;

    for (int i = 0; i < pipeline->command_count; i++)
    {
        status = execute_command(&pipeline->commands[i], should_exit);
        if (should_exit != NULL && *should_exit)
        {
            break;
        }
    }

    return status;
}
