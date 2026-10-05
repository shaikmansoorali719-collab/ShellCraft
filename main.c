#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "history.h"
#include "help.h"

int main(void)
{
    /* Display a welcome banner when the shell starts */
    printf("=====================================\n");
    printf("         ShellCraft\n");
    printf("  A Unix Style Shell Written in C\n");
    printf("=====================================\n");

    /* Initialize GNU Readline history */
    using_history();

    /* Load persistent history from ~/.shellcraft_history */
    load_history();

    char *line;

    while (1)
    {
        line = readline("shellcraft$ ");

        /* Ctrl+D / EOF */
        if (line == NULL)
        {
            printf("\nGoodbye!\n");
            break;
        }

        /* Empty input — just re-prompt */
        if (strlen(line) == 0)
        {
            free(line);
            continue;
        }

        /* Built-in: history */
        if (strcmp(line, "history") == 0)
        {
            print_history();
            free(line);
            continue;
        }

        /* Add to history */
        add_history(line);

        /* Built-in: exit */
        if (strcmp(line, "exit") == 0)
        {
            free(line);
            printf("Exiting...\n");
            break;
        }

        /* Built-in: help */
        if (handle_help(line))
        {
            free(line);
            continue;
        }

        /* Echo input (temporary — replaced by execution in M3) */
        printf("You entered: %s\n", line);

        free(line);
    }

    /* Save history to persistent file before exiting */
    save_history();

    return 0;
}
