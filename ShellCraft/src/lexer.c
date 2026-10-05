#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"

int lexer(const char *input, token_list_t *list)
{
    token_list_init(list);

    if (input == NULL)
        return 0;

    int i = 0;

    while (input[i] != '\0')
    {
        /* Skip whitespace */
        if (isspace((unsigned char)input[i]))
        {
            i++;
            continue;
        }

        /*
         * Feature 3: Comment handling
         * An unquoted, unescaped '#' outside quotes starts a shell comment.
         * Everything following '#' until end of line is ignored.
         */
        if (input[i] == '#')
        {
            break;
        }

        /* Check token capacity */
        if (list->count >= MAX_TOKENS - 1)
        {
            fprintf(stderr, "ShellCraft: syntax error: token limit exceeded\n");
            return 0;
        }

        /* Operator: Pipe | */
        if (input[i] == '|')
        {
            token_add(list, TOKEN_PIPE, "|");
            i++;
            continue;
        }

        /* Operator: Input redirection < */
        if (input[i] == '<')
        {
            token_add(list, TOKEN_INPUT, "<");
            i++;
            continue;
        }

        /* Operator: Output redirection > or Append >> */
        if (input[i] == '>')
        {
            if (input[i + 1] == '>')
            {
                token_add(list, TOKEN_APPEND, ">>");
                i += 2;
            }
            else
            {
                token_add(list, TOKEN_OUTPUT, ">");
                i++;
            }
            continue;
        }

        /* Operator: Background & */
        if (input[i] == '&')
        {
            token_add(list, TOKEN_BACKGROUND, "&");
            i++;
            continue;
        }

        /*
         * Build one WORD token.
         * We preserve quotes and escape markers so that the variable
         * expansion and syntax validation phases retain complete quote context.
         */
        char word[MAX_TOKEN_LEN];
        int j = 0;

        while (input[i] != '\0')
        {
            char c = input[i];

            /* End of word: stop at whitespace or operator outside quotes */
            if (isspace((unsigned char)c) ||
                c == '|' || c == '<' ||
                c == '>' || c == '&')
            {
                break;
            }

            /* Unescaped '#' outside quotes starts a comment */
            if (c == '#')
            {
                break;
            }

            /* Single quotes: preserve everything inside literally */
            if (c == '\'')
            {
                if (j < MAX_TOKEN_LEN - 1)
                    word[j++] = input[i++];

                while (input[i] != '\0' && input[i] != '\'')
                {
                    if (j < MAX_TOKEN_LEN - 1)
                        word[j++] = input[i++];
                    else
                        i++;
                }

                /* Feature 2: Syntax validation for unclosed single quote */
                if (input[i] != '\'')
                {
                    fprintf(stderr, "ShellCraft: syntax error: unterminated single quote\n");
                    return 0;
                }

                if (j < MAX_TOKEN_LEN - 1)
                    word[j++] = input[i++];
                continue;
            }

            /* Double quotes: preserve quotes and handle internal escapes */
            if (c == '"')
            {
                if (j < MAX_TOKEN_LEN - 1)
                    word[j++] = input[i++];

                while (input[i] != '\0' && input[i] != '"')
                {
                    if (input[i] == '\\' && input[i + 1] != '\0')
                    {
                        if (j < MAX_TOKEN_LEN - 1)
                            word[j++] = input[i++];
                    }

                    if (j < MAX_TOKEN_LEN - 1)
                        word[j++] = input[i++];
                    else
                        i++;
                }

                /* Feature 2: Syntax validation for unclosed double quote */
                if (input[i] != '"')
                {
                    fprintf(stderr, "ShellCraft: syntax error: unterminated double quote\n");
                    return 0;
                }

                if (j < MAX_TOKEN_LEN - 1)
                    word[j++] = input[i++];
                continue;
            }

            /* Escape character \ */
            if (c == '\\')
            {
                if (j < MAX_TOKEN_LEN - 1)
                    word[j++] = input[i++];

                if (input[i] != '\0')
                {
                    if (j < MAX_TOKEN_LEN - 1)
                        word[j++] = input[i++];
                }
                continue;
            }

            /* Normal character */
            if (j < MAX_TOKEN_LEN - 1)
                word[j++] = c;

            i++;
        }

        word[j] = '\0';
        token_add(list, TOKEN_WORD, word);
    }

    token_add(list, TOKEN_END, "END");
    return 1;
}
