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
#include "builtin.h"
#include "executor.h"

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

        /* Built-in: history (Milestone 1 behavior: not added to history) */
        if (strcmp(line, "history") == 0)
        {
            print_history();
            set_last_status(0);
            free(line);
            continue;
        }

        /* Add to history */
        add_history(line);

        /* Milestone 2: Lexical analysis & tokenization */
        if (!lexer(line, &tokens))
        {
            set_last_status(2); /* syntax error status */
            free(line);
            continue;
        }

        /* If only a comment was entered, token list only has TOKEN_END */
        if (tokens.count <= 1)
        {
            free(line);
            continue;
        }

        int should_exit = 0;
        int exit_code = 0;

        /* Milestone 2: Parsing & syntax validation */
        if (parser(&tokens, &pipeline))
        {
            /* Milestone 2: Advanced environment variable expansion (including $?) */
            expand_variables(&pipeline);

            /* Milestone 3: Process execution */
            exit_code = execute_pipeline(&pipeline, &should_exit);

            /* Free dynamically allocated memory in pipeline */
            pipeline_free(&pipeline);
        }
        else
        {
            set_last_status(2); /* syntax error status */
        }

        free(line);

        if (should_exit)
        {
            printf("Exiting...\n");
            save_history();
            return exit_code;
        }
    }

    /* Save history to persistent file before exiting */
    save_history();

    return get_last_status();
}
