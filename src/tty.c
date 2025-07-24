/* tty.c */

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

#include <stdio.h>
//#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>

#include "tty.h"

void
tty_reset(tty_t *tty)
{
	tcsetattr(tty->fdin, TCSANOW, &tty->original_termios);
}

void
tty_close(tty_t *tty)
{
	tty_reset(tty);
	fclose(tty->fout);
	close(tty->fdin);
}

static void
handle_sigwinch(int sig)
{
	(void)sig;
}

void
tty_init(tty_t *tty, const char *tty_filename)
{
	tty->fdin = open(tty_filename, O_RDONLY);
	if (tty->fdin < 0) {
		perror("Failed to open tty");
		exit(EXIT_FAILURE);
	}

	tty->fout = fopen(tty_filename, "w");
	if (!tty->fout) {
		perror("Failed to open tty");
		exit(EXIT_FAILURE);
	}

	if (setvbuf(tty->fout, NULL, _IOFBF, 16384)) {
		perror("setvbuf");
		exit(EXIT_FAILURE);
	}

	if (tcgetattr(tty->fdin, &tty->original_termios)) {
		perror("tcgetattr");
		exit(EXIT_FAILURE);
	}

	struct termios new_termios = tty->original_termios;

	 /* Disable all of
	 * ICANON  Canonical input (erase and kill processing).
	 * ECHO    Echo.
	 * ISIG    Signals from control characters.
	 * ICRNL   Conversion of CR characters into NL. */
	new_termios.c_iflag &= ~(ICRNL);
	new_termios.c_lflag &= ~(ICANON | ECHO | ISIG);

	if (tcsetattr(tty->fdin, TCSANOW, &new_termios))
		perror("tcsetattr");

	tty_getwinsz(tty);
	tty_setnormal(tty);
	signal(SIGWINCH, handle_sigwinch);
}

void
tty_getwinsz(tty_t *tty)
{
	struct winsize ws;
	if (ioctl(fileno(tty->fout), TIOCGWINSZ, &ws) == -1) {
		tty->maxwidth = DEFAULT_TERMINAL_COLS;
		tty->maxheight = DEFAULT_TERMINAL_LINES;
	} else {
		tty->maxwidth = ws.ws_col;
		tty->maxheight = ws.ws_row;
	}
}

char
tty_getchar(tty_t *tty)
{
	char ch;
	const int size = read(tty->fdin, &ch, 1);
	if (size < 0) {
		perror("error reading from tty");
		exit(EXIT_FAILURE);
	} else if (size == 0) {
		/* EOF */
		exit(EXIT_FAILURE);
	} else {
		return ch;
	}
}

int
tty_input_ready(tty_t *tty, const long int timeout, const int return_on_signal)
{
	fd_set readfs;
	FD_ZERO(&readfs);
	FD_SET(tty->fdin, &readfs);

	struct timespec ts = {timeout / 1000, (timeout % 1000) * 1000000};

	sigset_t mask;
	sigemptyset(&mask);
	if (return_on_signal == 0)
		sigaddset(&mask, SIGWINCH);

	const int err = pselect(tty->fdin + 1, &readfs, NULL, NULL,
		timeout < 0 ? NULL : &ts, return_on_signal == 1 ? NULL : &mask);

	if (err < 0) {
		if (errno == EINTR)
			return 0;

		perror("select");
		exit(EXIT_FAILURE);
	}

	return FD_ISSET(tty->fdin, &readfs);
}

static void
tty_sgr(const tty_t *tty, const int code)
{
	fprintf(tty->fout, "\x1b[%dm", code);
}

void
tty_setfg(tty_t *tty, const int fg)
{
	if (tty->fgcolor != fg) {
		tty_sgr(tty, 30 + fg);
		tty->fgcolor = fg;
	}
}

void
tty_setinvert(const tty_t *tty)
{
	tty_sgr(tty, 7);
}

void
tty_setunderline(const tty_t *tty)
{
	tty_sgr(tty, 4);
}

void
tty_setnormal(tty_t *tty)
{
	tty_sgr(tty, 0);
	tty->fgcolor = 9;
}

void
tty_setnowrap(const tty_t *tty)
{
	fputs("\x1b[?7l", tty->fout);
}

void
tty_setwrap(const tty_t *tty)
{
	fputs("\x1b[?7h", tty->fout);
}

void
tty_newline(const tty_t *tty)
{
	fputs("\x1b[K\n", tty->fout);
}

void
tty_clearline(const tty_t *tty)
{
	fputs("\x1b[K", tty->fout);
}

void
tty_setcol(const tty_t *tty, const int col)
{
	fprintf(tty->fout, "\x1b[%dG", col + 1);
}

void
tty_moveup(const tty_t *tty, const int i)
{
	fprintf(tty->fout, "\x1b[%dA", i);
}

void
tty_fputs(const tty_t *tty, const char *str)
{
	fputs(str, tty->fout);
}

void
tty_putc(const tty_t *tty, const char c)
{
	fputc(c, tty->fout);
}

void
tty_flush(const tty_t *tty)
{
	fflush(tty->fout);
}

void
tty_hide_cursor(const tty_t *tty)
{
	fputs("\x1b[?25l", tty->fout);
}

void
tty_unhide_cursor(const tty_t *tty)
{
	fputs("\x1b[?25h", tty->fout);
}

size_t
tty_getheight(const tty_t *tty)
{
	return tty->maxheight;
}

void
tty_printf(tty_t *tty, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(tty->fout, fmt, args);
	va_end(args);
}
