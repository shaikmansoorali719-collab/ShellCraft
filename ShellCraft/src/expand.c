#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "expand.h"
#include "executor.h"

/*
 * Dynamic string buffer for accumulating expanded characters safely.
 */
typedef struct {
    char *data;
    size_t len;
    size_t capacity;
} str_buf_t;

static void buf_init(str_buf_t *b)
{
    b->capacity = 128;
    b->data = malloc(b->capacity);
    b->len = 0;
    if (b->data)
        b->data[0] = '\0';
}

static void buf_append_char(str_buf_t *b, char c)
{
    if (b->data == NULL)
        return;

    if (b->len + 2 > b->capacity)
    {
        size_t new_cap = b->capacity * 2;
        char *new_data = realloc(b->data, new_cap);
        if (!new_data)
            return;
        b->data = new_data;
        b->capacity = new_cap;
    }

    b->data[b->len++] = c;
    b->data[b->len] = '\0';
}

static void buf_append_str(str_buf_t *b, const char *s)
{
    if (s == NULL || b->data == NULL)
        return;

    size_t slen = strlen(s);
    if (b->len + slen + 1 > b->capacity)
    {
        size_t new_cap = b->capacity * 2 + slen;
        char *new_data = realloc(b->data, new_cap);
        if (!new_data)
            return;
        b->data = new_data;
        b->capacity = new_cap;
    }

    memcpy(b->data + b->len, s, slen);
    b->len += slen;
    b->data[b->len] = '\0';
}

/*
 * Expands a single string token according to:
 *  - Single quotes: preserve literal text, no variable expansion
 *  - Double quotes: allow variable expansion, strip outer quotes
 *  - Escaping: \$ becomes literal $, \# becomes literal #
 *  - Embedded, multiple, and braced (${VAR}) variables
 */
char *expand_string(const char *str)
{
    if (str == NULL)
        return NULL;

    str_buf_t buf;
    buf_init(&buf);

    size_t i = 0;
    size_t len = strlen(str);

    while (i < len)
    {
        char c = str[i];

        /* Single quotes: copy everything literally until closing quote */
        if (c == '\'')
        {
            i++; /* skip opening single quote */
            while (i < len && str[i] != '\'')
            {
                buf_append_char(&buf, str[i]);
                i++;
            }
            if (i < len && str[i] == '\'')
            {
                i++; /* skip closing single quote */
            }
            continue;
        }

        /* Double quotes: perform expansion inside, handle escapes, strip quotes */
        if (c == '"')
        {
            i++; /* skip opening double quote */
            while (i < len && str[i] != '"')
            {
                /* Escaped characters inside double quotes */
                if (str[i] == '\\' && i + 1 < len)
                {
                    char next = str[i + 1];
                    if (next == '$' || next == '"' || next == '\\')
                    {
                        buf_append_char(&buf, next);
                        i += 2;
                        continue;
                    }
                }

                /* Variable expansion inside double quotes */
                if (str[i] == '$')
                {
                    i++; /* skip '$' */

                    if (i < len && str[i] == '?')
                    {
                        /* Previous command exit status: $? */
                        char status_str[32];
                        snprintf(status_str, sizeof(status_str), "%d", get_last_status());
                        buf_append_str(&buf, status_str);
                        i++;
                        continue;
                    }
                    else if (i < len && str[i] == '{')
                    {
                        /* Braced variable: ${VAR} or ${?} */
                        i++; /* skip '{' */
                        char var_name[128];
                        size_t v = 0;
                        while (i < len && str[i] != '}' && v < sizeof(var_name) - 1)
                        {
                            var_name[v++] = str[i++];
                        }
                        var_name[v] = '\0';

                        if (i < len && str[i] == '}')
                        {
                            i++; /* skip '}' */
                        }

                        if (strcmp(var_name, "?") == 0)
                        {
                            char status_str[32];
                            snprintf(status_str, sizeof(status_str), "%d", get_last_status());
                            buf_append_str(&buf, status_str);
                        }
                        else
                        {
                            const char *val = getenv(var_name);
                            if (val != NULL)
                            {
                                buf_append_str(&buf, val);
                            }
                        }
                        continue;
                    }
                    else if (i < len && (isalpha((unsigned char)str[i]) || str[i] == '_'))
                    {
                        /* Regular variable: $VAR */
                        char var_name[128];
                        size_t v = 0;
                        while (i < len && (isalnum((unsigned char)str[i]) || str[i] == '_') && v < sizeof(var_name) - 1)
                        {
                            var_name[v++] = str[i++];
                        }
                        var_name[v] = '\0';

                        const char *val = getenv(var_name);
                        if (val != NULL)
                        {
                            buf_append_str(&buf, val);
                        }
                        continue;
                    }
                    else
                    {
                        /* Standalone '$' */
                        buf_append_char(&buf, '$');
                        continue;
                    }
                }

                buf_append_char(&buf, str[i]);
                i++;
            }

            if (i < len && str[i] == '"')
            {
                i++; /* skip closing double quote */
            }
            continue;
        }

        /* Escape outside quotes */
        if (c == '\\')
        {
            i++; /* skip '\' */
            if (i < len)
            {
                buf_append_char(&buf, str[i]);
                i++;
            }
            continue;
        }

        /* Variable expansion outside quotes */
        if (c == '$')
        {
            i++; /* skip '$' */

            if (i < len && str[i] == '?')
            {
                /* Previous command exit status: $? */
                char status_str[32];
                snprintf(status_str, sizeof(status_str), "%d", get_last_status());
                buf_append_str(&buf, status_str);
                i++;
                continue;
            }
            else if (i < len && str[i] == '{')
            {
                /* Braced variable: ${VAR} or ${?} */
                i++; /* skip '{' */
                char var_name[128];
                size_t v = 0;
                while (i < len && str[i] != '}' && v < sizeof(var_name) - 1)
                {
                    var_name[v++] = str[i++];
                }
                var_name[v] = '\0';

                if (i < len && str[i] == '}')
                {
                    i++; /* skip '}' */
                }

                if (strcmp(var_name, "?") == 0)
                {
                    char status_str[32];
                    snprintf(status_str, sizeof(status_str), "%d", get_last_status());
                    buf_append_str(&buf, status_str);
                }
                else
                {
                    const char *val = getenv(var_name);
                    if (val != NULL)
                    {
                        buf_append_str(&buf, val);
                    }
                }
                continue;
            }
            else if (i < len && (isalpha((unsigned char)str[i]) || str[i] == '_'))
            {
                /* Regular variable: $VAR */
                char var_name[128];
                size_t v = 0;
                while (i < len && (isalnum((unsigned char)str[i]) || str[i] == '_') && v < sizeof(var_name) - 1)
                {
                    var_name[v++] = str[i++];
                }
                var_name[v] = '\0';

                const char *val = getenv(var_name);
                if (val != NULL)
                {
                    buf_append_str(&buf, val);
                }
                continue;
            }
            else
            {
                /* Standalone '$' */
                buf_append_char(&buf, '$');
                continue;
            }
        }

        /* Normal character */
        buf_append_char(&buf, c);
        i++;
    }

    return buf.data;
}

void expand_variables(pipeline_t *pipeline)
{
    if (pipeline == NULL)
        return;

    for (int i = 0; i < pipeline->command_count; i++)
    {
        command_t *cmd = &pipeline->commands[i];

        for (int j = 0; j < cmd->argc; j++)
        {
            if (cmd->argv[j] == NULL)
                continue;

            char *expanded = expand_string(cmd->argv[j]);
            if (expanded != NULL)
            {
                free(cmd->argv[j]);
                cmd->argv[j] = expanded;
            }
        }

        /* Expand redirection input filename if present */
        if (cmd->input[0] != '\0')
        {
            char *exp_in = expand_string(cmd->input);
            if (exp_in != NULL)
            {
                strncpy(cmd->input, exp_in, MAX_FILENAME - 1);
                cmd->input[MAX_FILENAME - 1] = '\0';
                free(exp_in);
            }
        }

        /* Expand redirection output filename if present */
        if (cmd->output[0] != '\0')
        {
            char *exp_out = expand_string(cmd->output);
            if (exp_out != NULL)
            {
                strncpy(cmd->output, exp_out, MAX_FILENAME - 1);
                cmd->output[MAX_FILENAME - 1] = '\0';
                free(exp_out);
            }
        }
    }
}
