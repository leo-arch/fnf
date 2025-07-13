/* colors.c */

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

#include <stdlib.h> /* getenv */
#include <string.h> /* strlen */

#include "colors.h"
#include "config.h" /* DEFAULT_COLORS */

char colors[COLOR_ITEMS_NUM][MAX_COLOR_LEN];

/* Parse colors taken from FNF_COLORS environment variable
 * Colors are parsed in strict order (see config.h)
 * Colors could be: 0-7 for normal colors, and b0-b7 for bold colors
 * Specific colors could be skipped using a dash ('-').
 * Colors are stored in the COLORS array using the same order defined in
 * config.h
 * These colors are applied in draw() and draw_match() functions in this file
 *
 * For example, "-b1b2-46" is read as follows:
 * -: no PROMPT color
 * b1: POINTER in bold red
 * b2: MARKER in bold green
 * -: no color for SELECTED ENTRY FOREGROUND
 * 4: SELECTED ENTRY BACKGROUND in blue
 * 6: MATCHING CHARACTERS in cyan
 * */
void
set_colors(tty_interface_t *state)
{
	char *p = getenv("NO_COLOR");
	if (p) {
		state->options->no_color = 1;
		return;
	}

	p = getenv("FNF_COLORS");
	if (!p || !*p)
		p = DEFAULT_COLORS;

	size_t bold = 0, count = 0;
	size_t i;
	for (i = 0; p[i] && count < COLOR_ITEMS_NUM; i++) {
		if (p[i] == 'b') {
			bold = 1;
			continue;
		}
		if (p[i] < '0' || p[i] > '7' || p[i] == '-') {
			*colors[count] = '\0';
			bold = 0;
			count++;
			continue;
		}

		/* 16 colors: 0-7 normal; b0-b7 bright */
		snprintf(colors[count], MAX_COLOR_LEN, "\x1b[%s%c%cm",
			bold == 1 ? "1;" : "",
			count == SEL_BG_COLOR ? '4' : '3',
			p[i]);

		bold = 0;
		count++;
	}
}

char *
decolor_name(const char *name)
{
	if (!name)
		return NULL;

	static char buf[PATH_MAX + 1];
	char *p = buf;

	size_t i = 0;
	const size_t name_len = strlen(name);

	while (i < name_len) {
		if (name[i] == KEY_ESC && name[i + 1] == '[') {
			/* Skip the escape sequence */
			while (i < name_len && name[i] != 'm')
				i++;

			/* Move past the 'm' */
			if (i < name_len)
				i++;
		} else {
			*p++ = name[i++];
		}
	}

	*p = '\0';
	return (p == buf) ? NULL : buf;
}

#define BUF_SIZE 4096
void
colorize_match(const tty_interface_t *state, const size_t *positions,
	const char *choice, const char *orig_color)
{
	tty_t *tty = state->tty;
	const int no_color = state->options->no_color;
	const char *hl = colors[MATCH_COLOR];
	static char buf[BUF_SIZE];
	size_t l = 0; /* Current buffer length */
	size_t p = 0;
	int in_match = 0; /* Track whether we are currently in a match */

	*buf = '\0';

	if (positions[p] != 0) {
		/* If the first character is not a match, set the original color */
		if (no_color == 1 || !orig_color || !*orig_color)
			l += snprintf(buf + l, BUF_SIZE - l, RESET_ATTR);
		else
			l += snprintf(buf + l, BUF_SIZE - l, "%s", orig_color);
	}

	for (size_t i = 0; choice[i]; i++) {
		const int is_match = (positions[p] == i);

		if (is_match) {
			if (!in_match) {
				if (no_color == 1)
					l += snprintf(buf + l, BUF_SIZE - l, UNDERLINE);
				else
					l += snprintf(buf + l, BUF_SIZE - l, "%s", hl); /* Highlight */
				in_match = 1; /* Transition from non-match to match */
			}
		} else {
			if (in_match) {
				if (no_color == 1 || !orig_color || !*orig_color)
					l += snprintf(buf + l, BUF_SIZE - l, RESET_ATTR);
				else
					l += snprintf(buf + l, BUF_SIZE - l, "%s", orig_color);
				in_match = 0; /* Transition from match to non-match */
			}
		}

		/* Add the character to the buffer */
		buf[l++] = (choice[i] == '\n') ? ' ' : choice[i];

		if (l >= BUF_SIZE - 1)
			break; /* Buffer is full, stop adding more characters */

		/* Move to the next position if we are at a match */
		if (is_match)
			p++;
	}

	buf[l] = '\0';
	tty_fputs(tty, buf);
	tty_setnormal(tty); /* Reset to normal after writing */
}

void
colorize_no_match(tty_t *tty, const int selected, const char *choice)
{
	if (selected == 0) {
		tty_fputs(tty, choice);
		return;
	}

	static char buf[BUF_SIZE];
	*buf = '\0';
	size_t l = 0;

	/* If selected, handle colors */
	if (*colors[SEL_FG_COLOR] || *colors[SEL_BG_COLOR]) {
		if (*colors[SEL_FG_COLOR])
			l += snprintf(buf + l, BUF_SIZE - l, "%s", colors[SEL_FG_COLOR]);
		if (*colors[SEL_BG_COLOR])
			l += snprintf(buf + l, BUF_SIZE - l, "%s", colors[SEL_BG_COLOR]);
	} else { /* If no specific colors, set invert */
		l += snprintf(buf + l, BUF_SIZE - l, INVERT);
	}

	/* Append the choice to the buffer and null-terminate the string. */
	l += snprintf(buf + l, BUF_SIZE - l, "%s", choice);
	buf[l] = '\0';

	tty_fputs(tty, buf);
}
#undef BUF_SIZE

const char *
get_original_color(const char *choice)
{
	static char orig_color[MAX_COLOR_LEN + 1];
	size_t i = 0;

	/* Iterate through the string until we find the ending character ('m')
	 * of the last contiguous SGR sequence. */
	while (choice[i] != '\0') {
		if (choice[i] == 'm' && choice[i + 1] != KEY_ESC) /* Stop copying after 'm' */
			break;
		orig_color[i] = choice[i];
		i++;
	}

	/* If 'm' was found, copy it and null-terminate */
	if (choice[i] == 'm') {
		orig_color[i] = choice[i];
		orig_color[i + 1] = '\0';
		return orig_color;
	}

	return NULL;
}
