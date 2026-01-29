/*
 * This file is part of fnf
 *
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2014-2022 John Hawthorn <john.hawthorn@gmail.com>
 * SPDX-FileCopyrightText: 2022-2026, L. Abramovich <leo.clifm@outlook.com>
*/

/* tty_interface.h */

#ifndef TTY_INTERFACE_H
#define TTY_INTERFACE_H

#ifdef __linux__
# include <linux/limits.h> /* PATH_MAX */
#else
# include <limits.h>
#endif /* __linux__ */

#include "choices.h"
#include "options.h"
#include "tty.h"

#ifndef PATH_MAX
# ifdef __linux__
#  define PATH_MAX 4096
# else
#  define PATH_MAX 1024
# endif /* __linux__ */
#endif /* PATH_MAX */

extern int g_case_sensitive;

#define SEARCH_SIZE_MAX 4096
#define SIG_INTERRUPT 130 /* 128 + SIGINT (usually 2) */
#define PENDING_INPUT_MAX 32

#define MAX_POINTER_LEN   256
#define MAX_INFO_LINE_LEN 256

#define PTR_CUR_SEL     0
#define PTR_CUR_NOSEL   1
#define PTR_NOCUR_SEL   2
#define PTR_NOCUR_NOSEL 3
#define PTR_TYPES_NUM   4

/* Time (in ms) to wait for additional bytes of an escape sequence */
#define KEYTIMEOUT 25

#define FORCE_REDRAW -1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char str[MAX_POINTER_LEN];
	size_t len;
} pointer_t;

typedef struct {
	size_t size; /* Size of the selections buffer */
	size_t selected; /* Number of currently selected entries */
} sel_t;

typedef struct {
	tty_t *tty;
	choices_t *choices;
	options_t *options;
	sel_t *selection;
	size_t cursor;
	int ambiguous_key_pending;
	int exit;
	int redraw;
	char search[SEARCH_SIZE_MAX + 1];
	char last_search[SEARCH_SIZE_MAX + 1];
	char input[PENDING_INPUT_MAX]; /* Pending input buffer */
} tty_interface_t;

int is_boundary(const char c);
void tty_interface_init(tty_interface_t *state, tty_t *tty,
	choices_t *choices, options_t *options, sel_t *selection);
int tty_interface_run(tty_interface_t *state);

#ifdef __cplusplus
}
#endif

#endif /* TTY_INTERFACE_H */
