/* tty_interface.c */

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

#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 700 /* wcwidth, wcswidth */
#endif

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h> /* wcwidth, wcswidth */

#include "colors.h"
#include "config.h"
#include "keybindings.h"
#include "match.h"
#include "tty_interface.h"
#include "selections.h"

int g_case_sensitive = -1;

int
is_boundary(const char c)
{
	return (~c & (1 << 7) || c & (1 << 6));
}

static int
contains_utf8(const char *str)
{
	const unsigned char *s = (const unsigned char *)str;

	while (*s) {
		if (*s >= 0x80)
			return 1;
		s++;
	}

	return 0;
}

/* A strlen implementation able to handle wide chars.
 * Return the number of columns required to print the string STR (instead
 * of the number of bytes required to store STR). */
static size_t
wc_xstrlen(const char *restrict str)
{
	wchar_t wbuf[PATH_MAX];
	const size_t len = mbstowcs(wbuf, str, (size_t)PATH_MAX);
	if (len == (size_t)-1) /* Invalid multi-byte sequence found */
		return 0;

	const int width = wcswidth(wbuf, len);
	if (width != -1)
		return (size_t)width;

	return 0; /* A non-printable wide char was found */
}

static size_t
get_cursor_position(const size_t start, const tty_interface_t *state)
{
	if (!*state->search)
		return start;

	int cursor_position = (int)start;
	const size_t cursor = state->cursor;
	const char *search = state->search;

	if (contains_utf8(search) == 0) {
		for (size_t i = 0; i < cursor && search[i]; i++)
			cursor_position += is_boundary(search[i]);
		return (size_t)cursor_position;
	}

	static wchar_t wbuf[SEARCH_SIZE_MAX * sizeof(wchar_t)];
	const size_t ret = mbstowcs(wbuf, search, SEARCH_SIZE_MAX);
	if (ret == (size_t)-1)
		return start;

	size_t wbuf_index = 0;

	for (size_t i = 0; i < cursor && search[i]; i++) {
		if (!is_boundary(search[i]))
			continue;

		const int w = wcwidth(wbuf[wbuf_index++]);
		if (w > 0)
			cursor_position += w;
	}

	return (size_t)cursor_position;
}

static void
print_score(const tty_t *tty, const score_t score, const int pad)
{
	if (score == SCORE_MIN || score == SCORE_MAX) {
		tty_printf(tty, "\x1b[%dG%s[     ]%s ",
			pad + 1, colors[SCORE_COLOR], RESET_ATTR);
	} else {
		tty_printf(tty, "\x1b[%dG%s[%5.2f]%s ",
			pad + 1, colors[SCORE_COLOR], score, RESET_ATTR);
	}
}

static void
draw_match(tty_interface_t *state, const char *choice, const int selected,
	const pointer_t *pointer)
{
	tty_t *tty = state->tty;
	const options_t *options = state->options;
	const char *search = state->last_search;

	static char sel_color[(MAX_COLOR_LEN * 2) + 1] = "";
	if (!*sel_color) {
		snprintf(sel_color, sizeof(sel_color), "%s",
			*colors[SEL_FG_COLOR] ? colors[SEL_FG_COLOR] : "");
	}

	static char original_color[MAX_COLOR_LEN + 1]; *original_color = '\0';
	char *orig_color = original_color;
	const char *dchoice = choice;
	if (*choice == KEY_ESC || strchr(choice, KEY_ESC))
		dchoice = decolor_name(choice, search ? original_color : NULL);
	else
		orig_color = *colors[FG_COLOR] ? colors[FG_COLOR] : NULL;

	score_t score = SCORE_MIN;
	static size_t positions[MATCH_MAX_LEN];
	if (search) {
		memset(positions, -1, sizeof(positions));
		score = match_positions(search, dchoice, &positions[0]);
	} else {
		positions[0] = (size_t)-1;
	}

	if (options->show_scores == 1)
		print_score(tty, score, options->pad);

	if (positions[0] == (size_t)-1) { /* No matching result (or no query). */
		colorize_no_match(tty, selected == 0 ? orig_color : sel_color,
			selected == 0 ? choice : dchoice, pointer);
	} else { /* We have matches (and a query). */
		colorize_match(state, positions, dchoice, selected == 0
			? orig_color : sel_color, pointer, selected);
	}
}

static pointer_t *
build_pointer(const int current, const int selected, const options_t *options)
{
	static pointer_t pointer[PTR_TYPES_NUM] = {0};

	static int pad = -1;
	if (pad == -1)
		/* If --show-scores, padding is already done by draw_match() */
		pad = options->show_scores == 1 ? 0 : options->pad;

	static char *gutter_color = NULL;
	if (!gutter_color)
		gutter_color =
			!IS_SGR0(colors[GUTTER_COLOR]) ? colors[GUTTER_COLOR] : "";

	/* Let's construct the pointer string only once */
	if (!*pointer[0].str) {
		/* Current (hovered) and selected */
		snprintf(pointer[PTR_CUR_SEL].str, MAX_POINTER_LEN, "%*s%s%s%s%s%s",
			pad, "", colors[SEL_BG_COLOR], colors[POINTER_COLOR],
			options->pointer, colors[MARKER_COLOR], options->marker);
		pointer[PTR_CUR_SEL].len = strlen(pointer[PTR_CUR_SEL].str);

		/* Current (hovered) and not selected */
		snprintf(pointer[PTR_CUR_NOSEL].str, MAX_POINTER_LEN, "%*s%s%s%s ",
			pad, "", colors[SEL_BG_COLOR], colors[POINTER_COLOR],
			options->pointer);
		pointer[PTR_CUR_NOSEL].len = strlen(pointer[PTR_CUR_NOSEL].str);

		/* Not current (not hovered) and selected */
		snprintf(pointer[PTR_NOCUR_SEL].str, MAX_POINTER_LEN, "%*s%s %s%s%s%s",
			pad, "", gutter_color, *gutter_color ? RESET_ATTR : "",
			colors[MARKER_COLOR], options->marker, RESET_ATTR);
		pointer[PTR_NOCUR_SEL].len = strlen(pointer[PTR_NOCUR_SEL].str);

		/* Not current (not hovered) and not selected */
		snprintf(pointer[PTR_NOCUR_NOSEL].str, MAX_POINTER_LEN, "%*s%s %s ",
			pad, "", gutter_color, *gutter_color ? RESET_ATTR : "");
		pointer[PTR_NOCUR_NOSEL].len = strlen(pointer[PTR_NOCUR_NOSEL].str);
	}

	if (current == 1)
		return &pointer[(selected == 1 ? PTR_CUR_SEL : PTR_CUR_NOSEL)];

	return &pointer[(selected == 1 ? PTR_NOCUR_SEL : PTR_NOCUR_NOSEL)];
}

static void
build_separator(const tty_interface_t *state, char *separator, const size_t size)
{
	const char *sep_str = state->options->separator;
	const size_t len = wc_xstrlen(sep_str);
	const size_t p = (size_t)state->options->pad;
	const size_t w = state->tty->maxwidth;

	const size_t maxwidth = p < w ? w - p : w;
	const size_t max_repeats =
		(maxwidth > 5 ? maxwidth - 5 : maxwidth) / (len > 0 ? len : 1);

	size_t repeat = 0;
	int bytes = snprintf(separator, size, " %s", colors[SEPARATOR_COLOR]);
	while (repeat++ < max_repeats) {
		int n = snprintf(separator + bytes, size - (size_t)bytes, "%s", sep_str);
		if (n < 0 || n >= (int)size - bytes)
			break;
		bytes += n;
	}
}

static void
print_info(const tty_interface_t *state, const choices_t *choices, const int pad,
	const size_t sel_num, const int reverse)
{
	static char selected[32];
	if (sel_num > 0)
		snprintf(selected, sizeof(selected), " (%zu)", sel_num);
	else
		*selected = '\0';

	static char separator[1024] = "";
	if (!*separator && state->options->separator)
		build_separator(state, separator, sizeof(separator));

	static char buf[MAX_INFO_LINE_LEN + sizeof(separator)];
	snprintf(buf, sizeof(buf), "%s\x1b[%dG%s%zu/%zu%s%s%s%s",
		reverse == 0 ? "\n" : "", pad, colors[INFO_COLOR],
		choices->available, choices->size, selected,
		separator, RESET_ATTR CLEAR_LINE, reverse == 1 ? "\n" : "");

	tty_fputs(state->tty, buf);
}

static size_t
get_starting_item(const choices_t *choices, const options_t *options)
{
	size_t start = 0;
	size_t items = options->num_lines;
	const size_t current_selection = choices->selection;
	const size_t available = choices->available;

	size_t scrolloff = (size_t)options->scrolloff;
	if (scrolloff == (size_t)-1) { /* --scroll-off=auto (default) */
		items = items < available ? items : available;
		scrolloff = items >> 1; /* items / 2 */
	}

	if (current_selection + scrolloff >= items) {
		start = current_selection + scrolloff - items + 1;
		if (start + items >= available && available > 0)
			start = available - items;
	}

	return start;
}

static void
draw(tty_interface_t *state)
{
	if (state->redraw == 0) {
		state->redraw = 1;
		return;
	}

	const tty_t *tty = state->tty;
	const choices_t *choices = state->choices;
	const options_t *options = state->options;
	const size_t num_lines = options->num_lines;
	const size_t sel_num = state->selection->selected;
	const size_t start = get_starting_item(choices, options);
	const int options_pad = options->pad;
	const int options_reverse = options->reverse;
	const size_t options_show_info = (size_t)options->show_info;

	tty_hide_cursor(tty);
	tty_setnowrap(tty);

	if (options_reverse == 0) {
		/* Set column, print prompt, and clear line. */
		tty_printf(tty, "\x1b[%dG%s%s%s%s%s%s", options_pad + 1,
			colors[PROMPT_COLOR], options->prompt, RESET_ATTR,
			colors[QUERY_COLOR], state->search, RESET_ATTR);

		if (options_show_info == 1)
			print_info(state, choices, options_pad + 1, sel_num, 0);
	} else if (num_lines + 1 + options_show_info >= tty->maxheight) {
		/* Fix the phantom lines issue present in some terminals. */
		tty_fputs(tty, "\x1b[A\r\x1b[K");
	}

	/* Print matches */
	for (size_t i = start; i < start + num_lines; i++) {
		if (options_reverse == 0)
			tty_putc(tty, '\n');

		const char *choice = choices_get(choices, i);
		if (choice) {
			const int selected = (sel_num > 0 && is_selected(choice));
			const int current = (i == choices->selection);
			const pointer_t *ptr = build_pointer(current, selected, options);

			draw_match(state, choice, current, ptr);
		} else {
			tty_fputs(tty, CLEAR_LINE);
		}

		if (options_reverse == 1)
			tty_putc(tty, '\n');
	}

	if (options_reverse == 0 && (num_lines + options_show_info) > 0)
		tty_moveup(tty, (int)(num_lines + options_show_info));

	if (options_reverse == 1 && options_show_info == 1)
		print_info(state, choices, options_pad + 1, sel_num, 1);

	/* Let's draw the prompt */
	static size_t prompt_len = (size_t)-1;
	if (prompt_len == (size_t)-1)
		prompt_len = wc_xstrlen(options->prompt);

	const size_t cursor_position =
		get_cursor_position(prompt_len + (size_t)options_pad + 1, state);

	tty_printf(tty, "\x1b[%dG%s%s%s%s%s%s\x1b[%zuG", options_pad + 1,
		colors[PROMPT_COLOR], options->prompt, RESET_ATTR,
		colors[QUERY_COLOR], state->search, RESET_ATTR CLEAR_LINE,
		cursor_position);

	tty_setwrap(tty);
	tty_unhide_cursor(tty);
	tty_flush(tty);
}

static int
has_uppercase(const char *s)
{
	while (*s) {
		if (isupper(*s))
			return 1;
		s++;
	}

	return 0;
}

static void
update_search(tty_interface_t *state)
{
	if (state->options->case_sens_mode == CASE_SMART)
		g_case_sensitive = has_uppercase(state->search);

	choices_search(state->choices, state->search, state->options->sort);
	strcpy(state->last_search, state->search);
}

static void
update_state(tty_interface_t *state)
{
	if (*state->last_search != *state->search
	|| strcmp(state->last_search, state->search) != 0) {
		update_search(state);
		if (state->options->reverse == 1) {
			/* Hide cursor and move it up. */
			tty_printf(state->tty, "\x1b[?25l\x1b[%zuA\n", 1 +
				state->options->num_lines + (size_t)state->options->show_info);
		}
		draw(state);
		/* Prevent a double draw when modifying the search string. */
//		state->redraw = 0;
//	} else {
//		state->redraw = 1;
	}
}

void
tty_interface_init(tty_interface_t *state, tty_t *tty, choices_t *choices,
	options_t *options, sel_t *selection)
{
	state->tty = tty;
	state->choices = choices;
	state->options = options;
	state->ambiguous_key_pending = 0;

	*state->input = '\0';
	*state->search = '\0';
	*state->last_search = '\0';

	state->cursor = 0;
	state->redraw = 1;
	state->exit = -1;
	state->selection = selection;

	if (options->init_search) {
		const size_t search_max = sizeof(state->search) - 1;
		strncpy(state->search, options->init_search, search_max); /* flawfinder: ignore */
		state->search[search_max - 1] = '\0';
		state->cursor = strlen(state->search);
	}

	update_search(state);
}

int
tty_interface_run(tty_interface_t *state)
{
	if (state->options->no_color == 0)
		set_colors(state);
	if (state->options->auto_lines == 1) {
		state->options->num_lines =
			tty_getheight(state->tty) - 1 - (size_t)state->options->show_info;
	}
	draw(state);

	char curr_char[2] = "";

	for (;;) {
		do {
			while (!tty_input_ready(state->tty, -1, 1)) {
				/* We received a signal (probably WINCH) */
				if (state->options->auto_lines) {
					tty_getwinsz(state->tty);
					state->options->num_lines = tty_getheight(state->tty) - 1;
				}
				draw(state);
			}

			curr_char[0] = tty_getchar(state->tty);
			handle_input(state, curr_char, 0);

			if (state->ambiguous_key_pending == 1)
				continue;

			if (state->exit >= 0) {
				free_selections(state);
				return state->exit;
			}

			if (state->options->reverse == 1 && state->redraw == 1) {
				/* Hide cursor and move it up. */
				tty_printf(state->tty, "\x1b[?25l\x1b[%zuA\n", 1 +
					state->options->num_lines + (size_t)state->options->show_info);
			}

			draw(state);
		} while (tty_input_ready(state->tty,
			state->ambiguous_key_pending ? KEYTIMEOUT : 0, 0));

		if (state->ambiguous_key_pending == 1) {
			handle_input(state, "", 1);

			if (state->exit >= 0) {
				free_selections(state);
				return state->exit;
			}
		}

		update_state(state);
	}

	return state->exit;
}
