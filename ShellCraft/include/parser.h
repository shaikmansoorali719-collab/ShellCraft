#ifndef PARSER_H
#define PARSER_H

#include "token.h"

#define MAX_COMMANDS  16
#define MAX_ARGS      32
#define MAX_FILENAME  128

/*
 * Structure representing one command in a pipeline
 */
typedef struct
{
    char *argv[MAX_ARGS];
    int argc;
    char input[MAX_FILENAME];
    char output[MAX_FILENAME];
    int append;
    int background;
} command_t;

/*
 * Structure representing an entire pipeline
 */
typedef struct
{
    command_t commands[MAX_COMMANDS];
    int command_count;
} pipeline_t;

/*
 * Parses a token list into a pipeline structure.
 * Performs robust syntax validation:
 *  - Leading/trailing pipes
 *  - Consecutive pipes
 *  - Missing filenames after redirections
 *  - Invalid redirection sequences
 * Returns 1 on success, 0 on syntax error.
 */
int parser(const token_list_t *tokens, pipeline_t *pipeline);

/* Print structured representation of the parsed pipeline */
void pipeline_print(const pipeline_t *pipeline);

/* Free dynamically allocated memory in pipeline */
void pipeline_free(pipeline_t *pipeline);

#endif
