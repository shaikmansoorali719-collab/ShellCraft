#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "builtin.h"
#include "history.h"
#include "help.h"

extern char **environ;

/* =========================================================
   BUILTIN: cd
   ========================================================= */

static int builtin_cd(const command_t *cmd)
{
    const char *directory;

    /* cd with no argument or ~ changes to HOME */
    if (cmd->argc == 1 || strcmp(cmd->argv[1], "~") == 0)
    {
        directory = getenv("HOME");
        if (directory == NULL || directory[0] == '\0')
        {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
    }
    else if (cmd->argc == 2)
    {
        directory = cmd->argv[1];
    }
    else
    {
        fprintf(stderr, "cd: too many arguments\n");
        return 1;
    }

    if (chdir(directory) != 0)
    {
        perror("cd");
        return 1;
    }

    return 0;
}

/* =========================================================
   BUILTIN: pwd
   ========================================================= */

static int builtin_pwd(const command_t *cmd)
{
    (void)cmd;
    char cwd[4096];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("pwd");
        return 1;
    }

    printf("%s\n", cwd);
    return 0;
}

/* =========================================================
   BUILTIN: echo
   ========================================================= */

static int builtin_echo(const command_t *cmd)
{
    for (int i = 1; i < cmd->argc; i++)
    {
        printf("%s", cmd->argv[i]);
        if (i < cmd->argc - 1)
        {
            putchar(' ');
        }
    }
    putchar('\n');
    return 0;
}

/* =========================================================
   BUILTIN: exit
   ========================================================= */

static int builtin_exit(const command_t *cmd, int *should_exit)
{
    if (should_exit != NULL)
    {
        *should_exit = 1;
    }

    if (cmd->argc > 1)
    {
        return atoi(cmd->argv[1]);
    }

    return 0;
}

/* =========================================================
   BUILTIN: export
   ========================================================= */

static int is_valid_identifier(const char *name)
{
    if (name == NULL || name[0] == '\0')
        return 0;

    if (!isalpha((unsigned char)name[0]) && name[0] != '_')
        return 0;

    for (size_t i = 1; name[i] != '\0'; i++)
    {
        if (!isalnum((unsigned char)name[i]) && name[i] != '_')
            return 0;
    }

    return 1;
}

static int builtin_export(const command_t *cmd)
{
    /* If no arguments, print all environment variables */
    if (cmd->argc == 1)
    {
        if (environ != NULL)
        {
            for (char **env = environ; *env != NULL; env++)
            {
                printf("%s\n", *env);
            }
        }
        return 0;
    }

    int ret = 0;

    for (int i = 1; i < cmd->argc; i++)
    {
        char *arg = cmd->argv[i];
        char *eq = strchr(arg, '=');

        if (eq == NULL)
        {
            /* Export without assignment: validate identifier name */
            if (!is_valid_identifier(arg))
            {
                fprintf(stderr, "shellcraft: export: '%s': not a valid identifier\n", arg);
                ret = 1;
            }
            continue;
        }

        /* Split into NAME and VALUE */
        size_t name_len = (size_t)(eq - arg);
        char name[128];
        if (name_len >= sizeof(name))
        {
            fprintf(stderr, "shellcraft: export: identifier name too long\n");
            ret = 1;
            continue;
        }

        strncpy(name, arg, name_len);
        name[name_len] = '\0';

        if (!is_valid_identifier(name))
        {
            fprintf(stderr, "shellcraft: export: '%s': not a valid identifier\n", arg);
            ret = 1;
            continue;
        }

        const char *value = eq + 1;

        if (setenv(name, value, 1) != 0)
        {
            perror("export");
            ret = 1;
        }
    }

    return ret;
}

/* =========================================================
   BUILTIN: unset
   ========================================================= */

static int builtin_unset(const command_t *cmd)
{
    if (cmd->argc < 2)
    {
        fprintf(stderr, "shellcraft: unset: not enough arguments\n");
        return 1;
    }

    int ret = 0;

    for (int i = 1; i < cmd->argc; i++)
    {
        if (!is_valid_identifier(cmd->argv[i]))
        {
            fprintf(stderr, "shellcraft: unset: '%s': not a valid identifier\n", cmd->argv[i]);
            ret = 1;
            continue;
        }

        if (unsetenv(cmd->argv[i]) != 0)
        {
            perror("unset");
            ret = 1;
        }
    }

    return ret;
}

/* =========================================================
   CHECK WHETHER COMMAND IS A BUILTIN
   ========================================================= */

int is_builtin(const command_t *cmd)
{
    if (cmd == NULL || cmd->argc == 0 || cmd->argv[0] == NULL)
    {
        return 0;
    }

    const char *name = cmd->argv[0];

    if (strcmp(name, "cd") == 0 ||
        strcmp(name, "pwd") == 0 ||
        strcmp(name, "echo") == 0 ||
        strcmp(name, "exit") == 0 ||
        strcmp(name, "export") == 0 ||
        strcmp(name, "unset") == 0 ||
        strcmp(name, "help") == 0 ||
        strcmp(name, "history") == 0)
    {
        return 1;
    }

    return 0;
}

/* =========================================================
   EXECUTE BUILTIN
   ========================================================= */

int execute_builtin(command_t *cmd, int *should_exit)
{
    if (cmd == NULL || cmd->argc == 0 || cmd->argv[0] == NULL)
    {
        return 1;
    }

    const char *name = cmd->argv[0];

    if (strcmp(name, "cd") == 0)
    {
        return builtin_cd(cmd);
    }
    if (strcmp(name, "pwd") == 0)
    {
        return builtin_pwd(cmd);
    }
    if (strcmp(name, "echo") == 0)
    {
        return builtin_echo(cmd);
    }
    if (strcmp(name, "exit") == 0)
    {
        return builtin_exit(cmd, should_exit);
    }
    if (strcmp(name, "export") == 0)
    {
        return builtin_export(cmd);
    }
    if (strcmp(name, "unset") == 0)
    {
        return builtin_unset(cmd);
    }
    if (strcmp(name, "help") == 0)
    {
        display_help(cmd->argc > 1 ? cmd->argv[1] : NULL);
        return 0;
    }
    if (strcmp(name, "history") == 0)
    {
        print_history();
        return 0;
    }

    return 1;
}
