#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static void command_init(command_t *cmd)
{
    cmd->argc = 0;
    cmd->input[0] = '\0';
    cmd->output[0] = '\0';
    cmd->append = 0;
    cmd->background = 0;

    for (int i = 0; i < MAX_ARGS; i++)
        cmd->argv[i] = NULL;
}

int parser(const token_list_t *tokens, pipeline_t *pipeline)
{
    if (tokens == NULL || tokens->count == 0)
        return 0;

    /* Check if token list contains only TOKEN_END */
    if (tokens->count == 1 && tokens->tokens[0].type == TOKEN_END)
        return 0;

    /* Feature 2: Syntax validation for pipe at beginning */
    if (tokens->tokens[0].type == TOKEN_PIPE)
    {
        fprintf(stderr, "ShellCraft: syntax error near '|'\n");
        return 0;
    }

    /*
     * Feature 2: Robust syntax validation passes
     *  - Consecutive pipes (ls || wc)
     *  - Trailing pipe (ls |)
     *  - Missing filename after redirection (cat <)
     *  - Invalid redirection sequences (cat < >)
     */
    for (int i = 0; i < tokens->count; i++)
    {
        const token_t *curr = &tokens->tokens[i];

        if (curr->type == TOKEN_PIPE)
        {
            if (i + 1 < tokens->count && tokens->tokens[i + 1].type == TOKEN_PIPE)
            {
                fprintf(stderr, "ShellCraft: syntax error near '||'\n");
                return 0;
            }
            if (i + 1 < tokens->count && tokens->tokens[i + 1].type == TOKEN_END)
            {
                fprintf(stderr, "ShellCraft: syntax error: expected command after '|'\n");
                return 0;
            }
        }
        else if (curr->type == TOKEN_INPUT || curr->type == TOKEN_OUTPUT || curr->type == TOKEN_APPEND)
        {
            if (i + 1 >= tokens->count || tokens->tokens[i + 1].type == TOKEN_END)
            {
                fprintf(stderr, "ShellCraft: syntax error: expected filename after '%s'\n", curr->text);
                return 0;
            }

            const token_t *next = &tokens->tokens[i + 1];
            if (next->type != TOKEN_WORD)
            {
                fprintf(stderr, "ShellCraft: syntax error near unexpected token '%s'\n", next->text);
                return 0;
            }
        }
        else if (curr->type == TOKEN_BACKGROUND)
        {
            /* '&' cannot appear as the first token without a preceding command */
            if (i == 0 || (i > 0 && tokens->tokens[i - 1].type == TOKEN_PIPE))
            {
                fprintf(stderr, "ShellCraft: syntax error: '&' must appear at the end of the command\n");
                return 0;
            }

            /* '&' must appear strictly at the very end of the command/pipeline */
            if (i + 1 >= tokens->count || tokens->tokens[i + 1].type != TOKEN_END)
            {
                fprintf(stderr, "ShellCraft: syntax error: '&' must appear at the end of the command\n");
                return 0;
            }
        }
    }

    /* Build pipeline */
    pipeline->command_count = 1;
    int current = 0;
    command_init(&pipeline->commands[0]);

    for (int i = 0; i < tokens->count; i++)
    {
        const token_t *t = &tokens->tokens[i];

        switch (t->type)
        {
        case TOKEN_WORD:
            if (pipeline->commands[current].argc >= MAX_ARGS - 1)
            {
                fprintf(stderr, "ShellCraft: too many arguments for command\n");
                pipeline_free(pipeline);
                return 0;
            }

            pipeline->commands[current].argv[pipeline->commands[current].argc++] = strdup(t->text);
            break;

        case TOKEN_INPUT:
            i++; /* Skip to filename */
            strncpy(pipeline->commands[current].input, tokens->tokens[i].text, MAX_FILENAME - 1);
            pipeline->commands[current].input[MAX_FILENAME - 1] = '\0';
            break;

        case TOKEN_OUTPUT:
            i++; /* Skip to filename */
            strncpy(pipeline->commands[current].output, tokens->tokens[i].text, MAX_FILENAME - 1);
            pipeline->commands[current].output[MAX_FILENAME - 1] = '\0';
            pipeline->commands[current].append = 0;
            break;

        case TOKEN_APPEND:
            i++; /* Skip to filename */
            strncpy(pipeline->commands[current].output, tokens->tokens[i].text, MAX_FILENAME - 1);
            pipeline->commands[current].output[MAX_FILENAME - 1] = '\0';
            pipeline->commands[current].append = 1;
            break;

        case TOKEN_BACKGROUND:
            for (int k = 0; k <= current; k++)
            {
                pipeline->commands[k].background = 1;
            }
            break;

        case TOKEN_PIPE:
            if (pipeline->commands[current].argc == 0)
            {
                fprintf(stderr, "ShellCraft: syntax error near unexpected token '|'\n");
                pipeline_free(pipeline);
                return 0;
            }

            pipeline->commands[current].argv[pipeline->commands[current].argc] = NULL;
            current++;

            if (current >= MAX_COMMANDS)
            {
                fprintf(stderr, "ShellCraft: too many commands in pipeline\n");
                pipeline_free(pipeline);
                return 0;
            }

            command_init(&pipeline->commands[current]);
            pipeline->command_count++;
            break;

        case TOKEN_END:
            break;
        }
    }

    pipeline->commands[current].argv[pipeline->commands[current].argc] = NULL;

    /* Validate pipeline: check for any command with 0 arguments */
    for (int k = 0; k < pipeline->command_count; k++)
    {
        if (pipeline->commands[k].argc == 0)
        {
            fprintf(stderr, "ShellCraft: syntax error: unexpected end of pipeline\n");
            pipeline_free(pipeline);
            return 0;
        }
    }

    return 1;
}

void pipeline_print(const pipeline_t *pipeline)
{
    printf("\n========== PIPELINE ==========\n");

    for (int i = 0; i < pipeline->command_count; i++)
    {
        const command_t *cmd = &pipeline->commands[i];

        printf("\nCommand %d\n", i + 1);
        printf("-----------------------------\n");

        printf("Arguments\n");

        for (int j = 0; j < cmd->argc; j++)
            printf("  argv[%d] = %s\n", j, cmd->argv[j]);

        printf("Input      : %s\n",
               strlen(cmd->input) ? cmd->input : "None");

        printf("Output     : %s\n",
               strlen(cmd->output) ? cmd->output : "None");

        printf("Append     : %s\n",
               cmd->append ? "Yes" : "No");

        printf("Background : %s\n",
               cmd->background ? "Yes" : "No");
    }

    printf("==============================\n");
}

void pipeline_free(pipeline_t *pipeline)
{
    for (int i = 0; i < pipeline->command_count; i++)
    {
        command_t *cmd = &pipeline->commands[i];
        for (int j = 0; j < cmd->argc; j++)
        {
            if (cmd->argv[j] != NULL)
            {
                free(cmd->argv[j]);
                cmd->argv[j] = NULL;
            }
        }
        cmd->argc = 0;
    }
    pipeline->command_count = 0;
}
