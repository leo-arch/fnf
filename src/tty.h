/* tty.h */

/*
 * This file is part of fnf
 *
 * Copyright
 * (C) 2014-2022 John Hawthorn <john.hawthorn@gmail.com>
 * (C) 2022-2025, L. Abramovich <leo.clifm@outlook.com>
 * All rights reserved.

* The MIT License (MIT)

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#ifndef TTY_H
#define TTY_H

#include <termios.h>

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

#define TTY_COLOR_BLACK 0
#define TTY_COLOR_RED 1
#define TTY_COLOR_GREEN 2
#define TTY_COLOR_YELLOW 3
#define TTY_COLOR_BLUE 4
#define TTY_COLOR_MAGENTA 5
#define TTY_COLOR_CYAN 6
#define TTY_COLOR_WHITE 7
#define TTY_COLOR_NORMAL 9

/* tty_newline
 * Move cursor to the beginning of the next line, clearing to the end of the
 * current line
 */
void tty_newline(const tty_t *tty);

/* tty_clearline
 * Clear to the end of the current line without advancing the cursor.
 */
void tty_clearline(const tty_t *tty);

void tty_moveup(const tty_t *tty, const int i);
void tty_setcol(const tty_t *tty, const int col);

void tty_hide_cursor(const tty_t *tty);
void tty_unhide_cursor(const tty_t *tty);

void tty_fputs(const tty_t *tty, const char *str);
void tty_printf(tty_t *tty, const char *fmt, ...);
void tty_putc(const tty_t *tty, const char c);
void tty_flush(const tty_t *tty);

size_t tty_getheight(const tty_t *tty);

#endif /* TTY_H */
