#ifndef LEXER_H
#define LEXER_H

#include "token.h"

/*
 * Converts an input string to tokens.
 * Handles:
 *  - Whitespace splitting
 *  - Operators: |, <, >, >>, &
 *  - Quote handling: single quotes, double quotes, unclosed quote validation
 *  - Escape character: \
 *  - Comments: # outside quotes, preserved inside quotes and when escaped
 *
 * Returns 1 on success, 0 on syntax error (e.g. unclosed quote).
 */
int lexer(const char *input, token_list_t *list);

#endif
