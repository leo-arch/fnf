/* tty_interface.h */

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

#ifndef TTY_INTERFACE_H
#define TTY_INTERFACE_H

#ifdef __linux__
# include <linux/limits.h> /* PATH_MAX */
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) \
|| defined(__DragonFly__) || defined(__APPLE__)
# include <sys/syslimits.h>
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

#define SEARCH_SIZE_MAX 4096
#define SIG_INTERRUPT 130 /* 128 + SIGINT (usually 2) */

/* Time (in ms) to wait for additional bytes of an escape sequence */
#define KEYTIMEOUT 25

#define FORCE_REDRAW -1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	tty_t *tty;
	choices_t *choices;
	options_t *options;
	size_t cursor;
	int ambiguous_key_pending;
	int exit;
	int redraw;
	char search[SEARCH_SIZE_MAX + 1];
	char last_search[SEARCH_SIZE_MAX + 1];
	char input[32]; /* Pending input buffer */
} tty_interface_t;

void tty_interface_init(tty_interface_t *state, tty_t *tty,
	choices_t *choices, options_t *options);
int tty_interface_run(tty_interface_t *state);

#ifdef __cplusplus
}
#endif

#endif /* TTY_INTERFACE_H */
