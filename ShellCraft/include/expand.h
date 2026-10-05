#ifndef EXPAND_H
#define EXPAND_H

#include "parser.h"

/*
 * Expand environment variables in argv[] and redirection paths
 * of all commands in the pipeline.
 *
 * Supports:
 *  - Embedded variables (e.g. /home/$USER/project)
 *  - Multiple variables (e.g. $HOME/$USER)
 *  - Braced variables (e.g. ${HOME}, ${USER}_project)
 *  - Quoting rules (single quotes prevent expansion; double quotes allow)
 *  - Escape rules (\$ prevents expansion and keeps literal $)
 */
void expand_variables(pipeline_t *pipeline);

/* Expand a single string, returning a newly allocated string */
char *expand_string(const char *str);

#endif
