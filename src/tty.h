/*
 * This file is part of fnf
 *
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2014-2022 John Hawthorn <john.hawthorn@gmail.com>
 * SPDX-FileCopyrightText: 2022-2026, L. Abramovich <leo.clifm@outlook.com>
*/

/* tty.h */

#ifndef TTY_H
#define TTY_H

#include <termios.h>

#define DEFAULT_TERMINAL_COLS  80
#define DEFAULT_TERMINAL_LINES 25

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	struct termios original_termios;
	FILE *fout;
	size_t maxwidth;
	size_t maxheight;
	int fgcolor;
	int fdin;
} tty_t;

void tty_reset(tty_t *tty);
void tty_close(tty_t *tty);
void tty_init(tty_t *tty, const char *tty_filename);
void tty_getwinsz(tty_t *tty);
char tty_getchar(tty_t *tty);
int tty_input_ready(tty_t *tty, const long int timeout,
	const int return_on_signal);

void tty_setfg(tty_t *tty, const int fg);
void tty_setinvert(const tty_t *tty);
void tty_setunderline(const tty_t *tty);
void tty_setnormal(tty_t *tty);
void tty_setnowrap(const tty_t *tty);
void tty_setwrap(const tty_t *tty);

/* Move cursor to the beginning of the next line, clearing to the end of the
 * current line. */
void tty_newline(const tty_t *tty);

/* Clear to the end of the current line without advancing the cursor. */
void tty_clearline(const tty_t *tty);

void tty_moveup(const tty_t *tty, const int i);
void tty_setcol(const tty_t *tty, const int col);

void tty_hide_cursor(const tty_t *tty);
void tty_unhide_cursor(const tty_t *tty);

void tty_fputs(const tty_t *tty, const char *str);
void tty_printf(const tty_t *tty, const char *fmt, ...);
void tty_putc(const tty_t *tty, const char c);
void tty_flush(const tty_t *tty);

size_t tty_getheight(const tty_t *tty);

#ifdef __cplusplus
}
#endif

#endif /* TTY_H */
