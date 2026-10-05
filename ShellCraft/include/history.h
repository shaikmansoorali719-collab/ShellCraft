#ifndef HISTORY_H
#define HISTORY_H

/* Load command history from persistent history file (~/.shellcraft_history) */
void load_history(void);

/* Save command history to persistent history file (~/.shellcraft_history) */
void save_history(void);

/* Display the command history list */
void print_history(void);

#endif
