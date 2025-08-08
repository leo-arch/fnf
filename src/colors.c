/* colors.c */

/*
 * This file is part of fnf
 *
 * Copyright (C) 2022-2025, L. Abramovich <leo.clifm@outlook.com>
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

static int
hex2int(const char *str)
{
	static const unsigned char hex_chars[256] = {
		['0'] = 0, ['1'] = 1, ['2'] = 2, ['3'] = 3, ['4'] = 4,
		['5'] = 5, ['6'] = 6, ['7'] = 7, ['8'] = 8, ['9'] = 9,
		['a'] = 10, ['b'] = 11, ['c'] = 12, ['d'] = 13, ['e'] = 14, ['f'] = 15,
		['A'] = 10, ['B'] = 11, ['C'] = 12, ['D'] = 13, ['E'] = 14, ['F'] = 15
	};

	/* int << 4 == int * 16 */
	return ((hex_chars[(unsigned char)str[0]] << 4)
		+ hex_chars[(unsigned char)str[1]]);
}

/* Disassemble the hex color HEX into attribute, R, G, and B values.
 * Based on https://mprog.wordpress.com/c/miscellaneous/convert-hexcolor-to-rgb-decimal */
int
get_rgb(const char *hex, int *attr, int *r, int *g, int *b)
{
	if (!hex || !*hex)
		return (-1);

	if (*hex == '#') {
		if (!hex[1])
			return (-1);
		hex++;
	}

	const char *h = hex;

	/* Convert 3-digits HEX to 6-digits */
	static char buf[9];
	if (h[0] && h[1] && h[2] && (!h[3] || h[3] == COLOR_FIELD_SEP) ) {
		buf[0] = buf[1] = h[0];
		buf[2] = buf[3] = h[1];
		buf[4] = buf[5] = h[2];

		if (!h[3] || !h[4]) {
			buf[6] = '\0';
		} else {
			buf[6] = COLOR_FIELD_SEP;
			buf[7] = h[4];
			buf[8] = '\0';
		}

		h = buf;
	}

	char tmp[3];
	tmp[2] = '\0';

	tmp[0] = h[0]; tmp[1] = h[1];
	*r = hex2int(tmp);

	tmp[0] = h[2]; tmp[1] = h[3];
	*g = hex2int(tmp);

	tmp[0] = h[4]; tmp[1] = h[5];
	*b = hex2int(tmp);

	*attr = -1; /* Attribute unset */
	if (h[6] == COLOR_FIELD_SEP && IS_DIGIT(h[7]) && !h[8])
		*attr = h[7] - '0';

	return 0;
}

/* Convert hex color HEX into RGB format (as an SGR escape sequence)
 * One color attribute can be added to the hex color as follows:
 * RRGGBB:[1-9], where 1-9 could be:
 * 1: Bold or increased intensity
 * 2: Faint, decreased intensity or dim
 * 3: Italic (Not widely supported)
 * 4: Underline
 * 5: Slow blink
 * 6: Rapid blink
 * 7: Reverse video or invert
 * 8: Conceal or hide (Not widely supported)
 * 9: Crossed-out or strike
 *
 * Example: ffaff00:4 -> 4;38;2;250;255;0
 *
 * This function does not validate hex, make sure to validate it yourself.
 */
static void
set_hex_color(const int code, const char *hex)
{
	int attr = -1, r = 0, g = 0, b = 0;
	if (get_rgb(hex, &attr, &r, &g, &b) == -1)
		return;

	/* This is just a workaround: disable attributes for highlight color
	 * to avoid losing the original color when the entry is selected. */
	if (code == HIGHLIGHT_COLOR)
		attr = -1;

	const int bgfg = IS_BG_COLOR(code) ? 48 : 38;
	const size_t l = sizeof(colors[code]);

	if (attr == -1) /* No attribute */
		snprintf(colors[code], l, "\x1b[%d;2;%d;%d;%dm", bgfg, r, g, b);
	else
		snprintf(colors[code], l, "\x1b[%d;%d;2;%d;%d;%dm", attr, bgfg, r, g, b);
}

static void
set_256_color(const int code, char *color)
{
	if (!color || !*color || !IS_DIGIT(*color))
		return;

	int attr = -1;
	char *field_sep = strchr(color, COLOR_FIELD_SEP);
	if (field_sep && field_sep[1]) {
		*field_sep = '\0';
		/* This is just a workaround: disable attributes for highlight color
		 * to avoid losing the original color when the entry is selected. */
		if (IS_DIGIT(field_sep[1]) && code != HIGHLIGHT_COLOR)
			attr = field_sep[1] - '0';
	}

	const int n = atoi(color);
	if (n < 0 || n > 255)
		return;

	const int bgfg = IS_BG_COLOR(code) ? 48 : 38;

	const size_t l = sizeof(colors[code]);
	if (attr == -1)
		snprintf(colors[code], l, "\x1b[%d;5;%dm", bgfg, n);
	else
		snprintf(colors[code], l, "\x1b[%d;%d;5;%dm", attr, bgfg, n);
}

static void
set_color(const int code, char *color)
{
	if (!color || !*color)
		return;

	if (*color == '-' && color[1] == '1' && !color[2])
		memcpy(colors[code], RESET_ATTR, sizeof(RESET_ATTR));
	else if (*color == '#')
		set_hex_color(code, color + 1);
	else
		set_256_color(code, color);
}

static void
parse_color_field(char *field)
{
	if (!field || !*field)
		return;

	struct color_fields_t {
		const char *name;
		size_t namelen;
		int code;
	};

	const struct color_fields_t fields[] = {
		{"hl:", 3, HIGHLIGHT_COLOR},
		{"marker:", 7, MARKER_COLOR},
		{"prompt:", 7, PROMPT_COLOR},
		{"pointer:", 8, POINTER_COLOR},
		{"sel-bg:", 7, SEL_BG_COLOR},
		{"sel-fg:", 7, SEL_FG_COLOR},
		{"info:", 5, INFO_COLOR},
		{"score:", 6, SCORE_COLOR},
		{"fg:", 3, FG_COLOR},
		{"query:", 6, QUERY_COLOR},
		{"gutter:", 7, GUTTER_COLOR},
		{"separator:", 10, SEPARATOR_COLOR},
		{NULL, 0, 0}
	};

	for (size_t i = 0; fields[i].name; i++) {
		if (*field == *fields[i].name
		&& strncmp(field, fields[i].name, fields[i].namelen) == 0)
			set_color(fields[i].code, field + fields[i].namelen);
	}
}

static void
parse_color_line(char *line)
{
	const char *delimiter = ", ";

	char *token = strtok(line, delimiter);
	while (token) {
		parse_color_field(token);
		token = strtok(NULL, delimiter);
	}
}

/* Set interface colors parsing a color line, taken either from the --color
 * options or from the FNF_COLORS environment variable. */
void
set_colors(tty_interface_t *state)
{
	char *env = getenv("NO_COLOR");
	if (env) {
		state->options->no_color = 1;
		return;
	}

	char def_colors[sizeof(DEFAULT_COLORS)];
	env = getenv("FNF_COLORS");
	if (!env || !*env) {
		memcpy(def_colors, DEFAULT_COLORS, sizeof(def_colors));
		env = def_colors;
	}

	parse_color_line(env);

	if (state->options->color && *state->options->color)
		parse_color_line(state->options->color);
}

/* Remove the initial SGR sequence from NAME and return the resulting string.
 * If COLOR_BUF is not NULL, the initial SGR sequence is copied into this
 * buffer, which must be large enough to hold an SGR sequence (MAX_COLOR_LEN). */
char *
decolor_name(const char *name, char *color_buf)
{
	if (!name)
		return NULL;

	static char buf[PATH_MAX + 1];
	char *p = buf;

	size_t i = 0;
	const size_t name_len = strlen(name);
	size_t sgr_end = (size_t)-1;

	while (i < name_len) {
		if (IS_SGR_START(name + i)) {
			/* Skip the escape sequence */
			while (i < name_len && name[i] != 'm')
				i++;

			/* Move past the 'm' */
			if (i < name_len)
				i++;
		} else {
			if (sgr_end == (size_t)-1)
				sgr_end = i; /* End of the initial SGR sequence */
			*p++ = name[i++];
		}
	}

	*p = '\0';

	if (color_buf != NULL) {
		/* Copy the removed color into the COLOR_BUF buffer. */
		if (sgr_end > 0 && sgr_end < MAX_COLOR_LEN) {
			strncpy(color_buf, name, sgr_end); /* flawfinder: ignore */
			color_buf[sgr_end] = '\0';
		} else {
			color_buf[0] = '\0';
		}
	}

	return buf;
}

#define BUF_SIZE 8192
void
colorize_match(const tty_interface_t *state, const size_t *positions,
	const char *name, const char *original_color, const char *pointer,
	const int selected)
{
	const int no_color = state->options->no_color;
	const char *highlight =
		no_color == 1 ? HIGHLIGHT_NOCOLOR : colors[HIGHLIGHT_COLOR];
	const char *orig_color = no_color == 1
		? (selected == 1 ? RESET_ATTR SELECTION_NOCOLOR : RESET_ATTR)
		: ((original_color && *original_color) ? original_color : "");

	size_t l = 0; /* Current buffer length */
	size_t p = 0; /* Position in match */
	int in_match = 0; /* Track whether we are currently in a match */

	static char buf[BUF_SIZE];
	l += snprintf(buf, sizeof(buf), "%s", pointer);

	if (positions[p] != 0) {
		/* If the first character is not a match, set the original color */
		l += snprintf(buf + l, sizeof(buf) - l, "%s", orig_color);
	} else if (selected == 1) {
		/* The first character is a match. Let's copy the selection color
		 * to extend wathever attribute it has to the first character. */
		l += snprintf(buf + l, sizeof(buf) - l, "%s", no_color == 1 ?
		SELECTION_NOCOLOR : colors[SEL_FG_COLOR]);
	}

	for (size_t i = 0; name[i]; i++) {
		const int is_match = (positions[p] == i);

		if (is_match) {
			if (!in_match) {
				l += snprintf(buf + l, sizeof(buf) - l, "%s", highlight);
				in_match = 1; /* Transition from non-match to match */
			}
		} else {
			if (in_match) {
				l += snprintf(buf + l, sizeof(buf) - l, "%s", orig_color);
				in_match = 0; /* Transition from match to non-match */
			}
		}

		/* Append the current character to the buffer */
		buf[l++] = (name[i] == '\n') ? ' ' : name[i];
		/* If a multi-byte character, append the remaining bytes */
		while (IS_UTF8_CONT_BYTE(name[i + 1]))
			buf[l++] = name[++i];

		if (l >= sizeof(buf) - 1)
			break; /* Buffer is full: stop adding more characters */

		/* Move to the next position if we are at a match */
		if (is_match)
			p++;
	}

	l += snprintf(buf + l, sizeof(buf) - l, "%s",
		(*orig_color && (no_color == 1 || !IS_SGR0(orig_color)))
		? RESET_ATTR CLEAR_LINE : CLEAR_LINE);

	if (l >= sizeof(buf)) l = sizeof(buf) - 1;
	buf[l] = '\0';

	state->tty->fgcolor = TERM_FG_COLOR_RESET;
	tty_fputs(state->tty, buf);
}

void
colorize_no_match(tty_t *tty, const char *sel_color, const char *name,
	const char *pointer)
{
	static char buf[BUF_SIZE];

	if (!sel_color || IS_SGR0(sel_color)) { /* The entry is not selected */
		snprintf(buf, sizeof(buf), "%s%s%s", pointer, name, CLEAR_LINE);
		tty_fputs(tty, buf);
		return;
	}

	/* If selected, handle colors. */
	size_t l = snprintf(buf, sizeof(buf), "%s%s%s%s",
		pointer,
		*sel_color ? sel_color : SELECTION_NOCOLOR,
		name,
		RESET_ATTR CLEAR_LINE);

	if (l >= sizeof(buf)) l = sizeof(buf) - 1;
	buf[l] = '\0';

	tty->fgcolor = TERM_FG_COLOR_RESET;
	tty_fputs(tty, buf);
}
#undef BUF_SIZE
