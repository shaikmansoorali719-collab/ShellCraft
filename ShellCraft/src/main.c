#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "history.h"
#include "help.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "expand.h"

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

    token_list_t tokens;
    pipeline_t pipeline;
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

        /* Milestone 2: Lexical analysis & tokenization */
        if (!lexer(line, &tokens))
        {
            free(line);
            continue;
        }

        /* If only a comment was entered, token list only has TOKEN_END */
        if (tokens.count <= 1)
        {
            free(line);
            continue;
        }

        /* Milestone 2: Parsing & syntax validation */
        if (parser(&tokens, &pipeline))
        {
            /* Milestone 2: Advanced environment variable expansion */
            expand_variables(&pipeline);

            /* Milestone 2: Display structured pipeline representation */
            pipeline_print(&pipeline);

            /* Free dynamically allocated memory in pipeline */
            pipeline_free(&pipeline);
        }

        free(line);
    }

    /* Save history to persistent file before exiting */
    save_history();

    return 0;
}
