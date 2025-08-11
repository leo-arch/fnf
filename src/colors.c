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
set_hex_color(const int code, const char *hex, const int no_bold)
{
	int attr = -1, r = 0, g = 0, b = 0;
	if (get_rgb(hex, &attr, &r, &g, &b) == -1)
		return;

	/* This is just a workaround: disable attributes for highlight color
	 * to avoid losing the original color when the entry is selected. */
	if (code == HIGHLIGHT_COLOR || (no_bold == 1 && attr == 1))
		attr = -1;

	const int bgfg = IS_BG_COLOR(code) ? 48 : 38;
	const size_t l = sizeof(colors[code]);

	if (attr == -1) /* No attribute */
		snprintf(colors[code], l, "\x1b[%d;2;%d;%d;%dm", bgfg, r, g, b);
	else
		snprintf(colors[code], l, "\x1b[%d;%d;2;%d;%d;%dm", attr, bgfg, r, g, b);
}

static void
set_16_color(const int code, const int color, const int attr)
{
	const int bg = IS_BG_COLOR(code);

	int n = 0;
	if (color < 8) /* Standard ANSI colors (3-bit) */
		n = color + (bg == 1 ? 40 : 30);
	else /* High-intensity or bright colors (4-bit) */
		n = color - 8 + (bg == 1 ? 100 : 90);

	const size_t l = sizeof(colors[code]);
	if (attr == -1)
		snprintf(colors[code], l, "\x1b[%dm", n);
	else
		snprintf(colors[code], l, "\x1b[%d;%dm", attr, n);
}

static void
set_256_color(const int code, const int color, const int attr)
{
	const int bgfg = IS_BG_COLOR(code) ? 48 : 38;

	const size_t l = sizeof(colors[code]);
	if (attr == -1)
		snprintf(colors[code], l, "\x1b[%d;5;%dm", bgfg, color);
	else
		snprintf(colors[code], l, "\x1b[%d;%d;5;%dm", attr, bgfg, color);
}

static void
set_ansi_color(const int code, char *color, const int no_bold)
{
	if (!color || !*color || !IS_DIGIT(*color))
		return;

	int attr = -1;
	char *field_sep = strchr(color, COLOR_FIELD_SEP);
	if (field_sep && field_sep[1]) {
		*field_sep = '\0';
		/* This is just a workaround: disable attributes for highlight color
		 * to avoid losing the original color when the entry is selected. */
		if (IS_DIGIT(field_sep[1]) && code != HIGHLIGHT_COLOR) {
			attr = field_sep[1] - '0';
			if (no_bold == 1 && attr == 1)
				attr = -1;
		}
	}

	const int n = atoi(color);
	if (n < 0 || n > 255)
		return;

	if (n <= 15)
		set_16_color(code, n, attr);
	else
		set_256_color(code, n, attr);
}

static void
set_color(const int code, char *color, const int no_bold)
{
	if (!color || !*color)
		return;

	if (*color == '-' && color[1] == '1' && !color[2])
		memcpy(colors[code], RESET_ATTR, sizeof(RESET_ATTR));
	else if (*color == '#')
		set_hex_color(code, color + 1, no_bold);
	else
		set_ansi_color(code, color, no_bold);
}

static void
parse_color_field(char *field, const int no_bold)
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
		{"ghost:", 6, GHOST_COLOR},
		{NULL, 0, 0}
	};

	for (size_t i = 0; fields[i].name; i++) {
		if (*field == *fields[i].name
		&& strncmp(field, fields[i].name, fields[i].namelen) == 0)
			set_color(fields[i].code, field + fields[i].namelen, no_bold);
	}
}

static void
parse_color_line(char *line, const int no_bold)
{
	const char *delimiter = ", ";

	char *token = strtok(line, delimiter);
	while (token) {
		parse_color_field(token, no_bold);
		token = strtok(NULL, delimiter);
	}
}

static void
load_default_colors(const char *scheme, const int no_bold)
{
	/* A full color scheme using hex colors take less than 180 bytes. */
	char def_colors[512];

	if (!scheme || !*scheme)
		snprintf(def_colors, sizeof(def_colors), "%s", DEFAULT_COLORS_DARK);
	else if (strcmp(scheme, "light") == 0)
		snprintf(def_colors, sizeof(def_colors), "%s", DEFAULT_COLORS_LIGHT);
	else if (strcmp(scheme, "16") == 0)
		snprintf(def_colors, sizeof(def_colors), "%s", DEFAULT_COLORS_16);
	else
		snprintf(def_colors, sizeof(def_colors), "%s", DEFAULT_COLORS_DARK);

	parse_color_line(def_colors, no_bold);
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

	load_default_colors(state->options->color_scheme, state->options->no_bold);

	if ((env = getenv("FNF_COLORS")) && *env)
		parse_color_line(env, state->options->no_bold);

	if (state->options->color && *state->options->color)
		parse_color_line(state->options->color, state->options->no_bold);
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
			memcpy(color_buf, name, sgr_end);
			color_buf[sgr_end] = '\0';
		} else {
			color_buf[0] = '\0';
		}
	}

	return buf;
}

/* Append STR to BUF provided there is enough space in BUF to hold STR,
 * including the NUL terminator.
 * Returns the lenght of STR in case of success. Otherwise, zero is returned
 * and BUF is left unmodified. */
static inline size_t
append_str(char *buf, const size_t buf_space, const char *str,
	const size_t str_len)
{
	if (str_len + 1 >= buf_space)
		return 0;

	memcpy(buf, str, str_len + 1);
	return str_len;
}

#define BUF_SIZE 8192
/* Build a complete interface line and print it to the output device (STATE->TTY).
 * Prepend the pointer string POINTER->STR, and colorize the string NAME,
 * highlighting matching characters (according to POSITIONS) with the
 * appropriate color. The color of the original item, ORIGINAL_COLOR,
 * is preserved). */
void
colorize_match(const tty_interface_t *state, const size_t *positions,
	const char *name, const char *original_color, const pointer_t *pointer,
	const int selected)
{
	const int no_color = state->options->no_color;
	const char *highlight =
		no_color == 1 ? HIGHLIGHT_NOCOLOR : colors[HIGHLIGHT_COLOR];
	const char *orig_color = no_color == 1
		? (selected == 1 ? RESET_ATTR SELECTION_NOCOLOR : RESET_ATTR)
		: ((original_color && *original_color) ? original_color : "");
	const char *sel_color = no_color == 1
		? SELECTION_NOCOLOR : colors[SEL_FG_COLOR];

	const size_t oc_len = strlen(orig_color);

	/* Static lengths: let's calculate them only once. */
	static size_t clr_len = sizeof(CLEAR_LINE) - 1;
	static size_t reset_clr_len =
		(sizeof(RESET_ATTR) - 1) + (sizeof(CLEAR_LINE) - 1);
	static size_t hl_len = (size_t)-1;
	static size_t sel_color_len = 0;
	if (hl_len == (size_t)-1) {
		hl_len = strlen(highlight);
		sel_color_len = strlen(sel_color);
	}

	size_t l = 0; /* Current buffer length */
	size_t p = 0; /* Position in match */
	int in_match = 0; /* Track whether we are currently in a match */

	static char buf[BUF_SIZE];
	l += append_str(buf, sizeof(buf), pointer->str, pointer->len);

	if (positions[p] != 0) {
		/* If the first character is not a match, set the original color */
		l += append_str(buf + l, sizeof(buf) - l, orig_color, oc_len);
	} else if (selected == 1) {
		/* The first character is a match. Let's copy the selection color
		 * to extend wathever attribute it has to the first character. */
		l += append_str(buf + l, sizeof(buf) - l, sel_color, sel_color_len);
	}

	for (size_t i = 0; name[i]; i++) {
		const int is_match = (positions[p] == i);

		if (is_match) {
			if (!in_match) {
				l += append_str(buf + l, sizeof(buf) - l, highlight, hl_len);
				in_match = 1; /* Transition from non-match to match */
			}
			p++; /* Move to the next position */
		} else {
			if (in_match) {
				l += append_str(buf + l, sizeof(buf) - l, orig_color, oc_len);
				in_match = 0; /* Transition from match to non-match */
			}
		}

		/* Make sure there is enough space in the buffer to write a complete
		 * UTF-8 character (max 4 bytes + NUL terminator). */
		if (l >= sizeof(buf) - 5)
			break;

		/* Append the current character to the buffer */
		buf[l++] = (name[i] == '\n') ? ' ' : name[i];
		/* If a multi-byte character, append the remaining bytes */
		while (IS_UTF8_CONT_BYTE(name[i + 1]))
			buf[l++] = name[++i];
	}

	const int reset = (*orig_color && (no_color == 1 || !IS_SGR0(orig_color)));
	l += append_str(buf + l, sizeof(buf) - l,
		reset == 1 ? RESET_ATTR CLEAR_LINE : CLEAR_LINE,
		reset == 1 ? reset_clr_len : clr_len);

	if (l >= sizeof(buf)) l = sizeof(buf) - 1;
	buf[l] = '\0';

	state->tty->fgcolor = TERM_FG_COLOR_RESET;
	tty_fputs(state->tty, buf);
}

/* Same as colorize_match, but for non-matching items. */
void
colorize_no_match(tty_t *tty, const char *sel_color, const char *name,
	const pointer_t *pointer)
{
	static char buf[BUF_SIZE];

	if (!sel_color || IS_SGR0(sel_color)) { /* The entry is not selected */
		snprintf(buf, sizeof(buf), "%s%s%s", pointer->str, name, CLEAR_LINE);
		tty_fputs(tty, buf);
		return;
	}

	/* If selected, handle colors. */
	int l = snprintf(buf, sizeof(buf), "%s%s%s%s",
		pointer->str,
		*sel_color ? sel_color : SELECTION_NOCOLOR,
		name,
		RESET_ATTR CLEAR_LINE);

	static int buf_size = (int)sizeof(buf);
	if (l >= buf_size) l = buf_size - 1;
	buf[l] = '\0';

	tty->fgcolor = TERM_FG_COLOR_RESET;
	tty_fputs(tty, buf);
}
#undef BUF_SIZE
