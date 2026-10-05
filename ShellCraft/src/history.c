#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <readline/history.h>

#include "history.h"

#define HISTORY_FILENAME ".shellcraft_history"

/*
 * Safely determine the absolute path to the user's history file.
 * Returns a dynamically allocated string containing the path, or NULL on failure.
 * The caller is responsible for freeing the returned string.
 */
static char *get_history_filepath(void)
{
    const char *home = getenv("HOME");

    /* Fallback to passwd entry if HOME environment variable is not set */
    if (home == NULL || home[0] == '\0')
    {
        struct passwd *pw = getpwuid(getuid());
        if (pw != NULL)
        {
            home = pw->pw_dir;
        }
    }

    if (home == NULL || home[0] == '\0')
    {
        return NULL;
    }

    size_t home_len = strlen(home);
    size_t filename_len = strlen(HISTORY_FILENAME);
    /* Allocate space for home + '/' (if needed) + filename + '\0' */
    size_t total_len = home_len + filename_len + 2;

    char *path = malloc(total_len);
    if (path == NULL)
    {
        return NULL;
    }

    if (home[home_len - 1] == '/')
    {
        snprintf(path, total_len, "%s%s", home, HISTORY_FILENAME);
    }
    else
    {
        snprintf(path, total_len, "%s/%s", home, HISTORY_FILENAME);
    }

    return path;
}

/*
 * Load previous commands from ~/.shellcraft_history into readline's history list.
 * Gracefully handles missing files, permission errors, or unresolvable HOME directory.
 */
void load_history(void)
{
    char *path = get_history_filepath();
    if (path == NULL)
    {
        return;
    }

    /*
     * read_history() returns 0 on success, or errno on failure.
     * If the file does not exist yet (ENOENT) or cannot be read,
     * the error is safely ignored and the shell continues normally.
     */
    read_history(path);

    free(path);
}

/*
 * Save command history to ~/.shellcraft_history.
 * Gracefully handles write errors, unresolvable HOME, or missing files.
 */
void save_history(void)
{
    char *path = get_history_filepath();
    if (path == NULL)
    {
        return;
    }

    /*
     * write_history() overwrites the file with current history lines,
     * creating the file if it does not exist yet.
     */
    write_history(path);

    free(path);

    /* Free memory allocated by GNU Readline history entries */
    clear_history();
}

/* Display the command history list */
void print_history(void)
{
    HIST_ENTRY **list = history_list();

    if (list == NULL)
    {
        printf("History is empty.\n");
        return;
    }

    printf("\n------ Command History ------\n");

    for (int i = 0; list[i] != NULL; i++)
    {
        printf("%2d  %s\n",
               i + history_base,
               list[i]->line);
    }

    printf("-----------------------------\n");
}
