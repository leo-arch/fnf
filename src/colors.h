/* colors.h */

/*
 * This file is part of fnf
 *
 * Copyright
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

#ifndef COLORS_H
#define COLORS_H

/* Color indices: colors (from FNF_COLORS env var) will be parsed
 * exactly in this order by set_colors(). */
#define PROMPT_COLOR  0
#define POINTER_COLOR 1
#define MARKER_COLOR  2
#define SEL_FG_COLOR  3
#define SEL_BG_COLOR  4
#define MATCH_COLOR   5
#define COLOR_ITEMS_NUM 6
#define MAX_COLOR_LEN 48

#define RESET_ATTR "\x1b[0m" /* Reset attributes */
#define CLEAR_LINE "\x1b[K"
#define UNDERLINE  "\x1b[4m"
#define INVERT     "\x1b[7m"

#define KEY_ESC 27
#define IS_SGR_CHAR(c) (((c) >= '0' && (c) <= '9') || (c) == ';' || (c) == '[')
#define IS_SGR_START(s) (*(s) == KEY_ESC && (s)[1] == '[')

#include "tty_interface.h"

extern char colors[COLOR_ITEMS_NUM][MAX_COLOR_LEN];

char *decolor_name(const char *name);
void colorize_match(const tty_interface_t *state, const size_t *positions,
	const char *name, const char *orig_color);
void colorize_no_match(tty_t *tty, const int selected, const char *name);
const char *get_original_color(const char *name);
void set_colors(tty_interface_t *state);

#endif /* COLORS_H */
