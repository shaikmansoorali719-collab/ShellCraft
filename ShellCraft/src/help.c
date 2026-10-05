#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "help.h"

/*
 * Built-in command registry.
 * To add support for new built-in commands in future milestones,
 * simply add a new entry to this table.
 */
static const builtin_cmd_t BUILTIN_COMMANDS[] = {
    {
        .name = "help",
        .summary = "Display help information",
        .description = "Display information about builtin commands.\n"
                       "Displays a list of all built-in commands, or detailed\n"
                       "help for a specific command if an argument is given.",
        .usage = "help [command]"
    },
    {
        .name = "history",
        .summary = "Display command history",
        .description = "Displays previously executed commands.",
        .usage = "history"
    },
    {
        .name = "exit",
        .summary = "Exit ShellCraft",
        .description = "Exits the ShellCraft session.",
        .usage = "exit"
    }
};

static const size_t NUM_BUILTINS = sizeof(BUILTIN_COMMANDS) / sizeof(BUILTIN_COMMANDS[0]);

/*
 * Display general help or command-specific help.
 */
void display_help(const char *cmd)
{
    if (cmd == NULL || cmd[0] == '\0')
    {
        printf("\n================= ShellCraft Help =================\n\n");
        printf("Built-in Commands:\n\n");

        for (size_t i = 0; i < NUM_BUILTINS; i++)
        {
            printf("  %-10s %s\n", BUILTIN_COMMANDS[i].name, BUILTIN_COMMANDS[i].summary);
        }

        printf("\nUsage:\n");
        printf("  help <command>\n\n");
        printf("====================================================\n\n");
        return;
    }

    /* Look up command in built-in table */
    for (size_t i = 0; i < NUM_BUILTINS; i++)
    {
        if (strcmp(BUILTIN_COMMANDS[i].name, cmd) == 0)
        {
            printf("\n%s\n", BUILTIN_COMMANDS[i].name);

            size_t name_len = strlen(BUILTIN_COMMANDS[i].name);
            for (size_t j = 0; j < name_len; j++)
            {
                putchar('-');
            }
            putchar('\n');

            printf("%s\n\n", BUILTIN_COMMANDS[i].description);
            printf("Usage:\n");
            printf("    %s\n\n", BUILTIN_COMMANDS[i].usage);
            return;
        }
    }

    /* Command not found */
    printf("ShellCraft: no help available for '%s'\n", cmd);
}

/*
 * Check if the input line is a 'help' invocation and handle it.
 * Returns 1 if handled, 0 otherwise.
 */
int handle_help(const char *cmd_line)
{
    if (cmd_line == NULL)
    {
        return 0;
    }

    /* Skip leading whitespace */
    const char *p = cmd_line;
    while (*p == ' ' || *p == '\t')
    {
        p++;
    }

    /* Must start with "help" */
    if (strncmp(p, "help", 4) != 0)
    {
        return 0;
    }

    /* Must be followed by end of string or whitespace */
    p += 4;
    if (*p != '\0' && *p != ' ' && *p != '\t')
    {
        return 0;
    }

    /* Skip whitespace before argument */
    while (*p == ' ' || *p == '\t')
    {
        p++;
    }

    if (*p == '\0')
    {
        display_help(NULL);
        return 1;
    }

    /* Extract the argument (command name) */
    char arg[64];
    size_t i = 0;
    while (*p != '\0' && *p != ' ' && *p != '\t' && i < sizeof(arg) - 1)
    {
        arg[i++] = *p++;
    }
    arg[i] = '\0';

    display_help(arg);
    return 1;
}
