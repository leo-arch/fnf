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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h> /* wc_xstrlen() */

#include "colors.h"
#include "config.h"
#include "match.h"
#include "tty_interface.h"
#include "selections.h"

static int
isprint_unicode(const char c)
{
	return (isprint(c) || c & (1 << 7));
}

static int
is_boundary(const char c)
{
	return (~c & (1 << 7) || c & (1 << 6));
}

static void
clear(const tty_interface_t *state)
{
	tty_t *tty = state->tty;

	tty_setcol(tty, state->options->pad);
	size_t line = 0;
	while (line++ < state->options->num_lines + (state->options->show_info ? 1 : 0))
		tty_newline(tty);

	tty_clearline(tty);
	if (state->options->num_lines > 0)
		tty_moveup(tty, line - 1);

	tty_flush(tty);
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

static int
contains_utf8(const char *str)
{
	while (*str) {
		if ((unsigned char)*str >= 0x80)
			return 1;
		str++;
	}

	return 0;
}

static size_t
get_cursor_position(const size_t base, tty_interface_t *state)
{
	if (!*state->search)
		return base;

	size_t cursor_position = base;

	const int is_utf8 = contains_utf8(state->search);
	if (is_utf8 == 0) {
		for (size_t i = 0; i < state->cursor && state->search[i]; i++)
			cursor_position += is_boundary(state->search[i]);
		return cursor_position;
	}

	static wchar_t wbuf[SEARCH_SIZE_MAX * sizeof(wchar_t)];
	const size_t ret = mbstowcs(wbuf, state->search, SEARCH_SIZE_MAX);
	if (ret == (size_t)-1)
		return base;

	size_t wbuf_index = 0;

	for (size_t i = 0; i < state->cursor && state->search[i]; i++) {
		if (!is_boundary(state->search[i]))
			continue;

		const int w = wcwidth(wbuf[wbuf_index++]);
		if (w > 0)
			cursor_position += w;
	}

	return cursor_position;
}

static void
draw_match(tty_interface_t *state, const char *choice, const int selected,
	const char *pointer)
{
	tty_t *tty = state->tty;
	options_t *options = state->options;
	const char *search = state->last_search;

	static char sel_color[(MAX_COLOR_LEN * 2) + 1] = "";
	if (!*sel_color) {
		snprintf(sel_color, sizeof(sel_color), "%s%s",
			*colors[SEL_FG_COLOR] ? colors[SEL_FG_COLOR] : "",
			*colors[SEL_BG_COLOR] ? colors[SEL_BG_COLOR] : "");
	}

	static char orig_color[MAX_COLOR_LEN + 1];
	*orig_color = '\0';
	const char *dchoice = choice;
	if (*choice == KEY_ESC || strchr(choice, KEY_ESC))
		dchoice = decolor_name(choice, search ? orig_color : NULL);

	score_t score = SCORE_MIN;
	static size_t positions[MATCH_MAX_LEN];
	if (search) {
		memset(positions, -1, sizeof(positions));
		score = match_positions(search, dchoice, &positions[0]);
	} else {
		positions[0] = (size_t)-1;
	}

	if (options->show_scores == 1) {
		if (score == SCORE_MIN)
			tty_fputs(tty, "(     ) ");
		else
			tty_printf(tty, "(%5.2f) ", score);
	}

	if (positions[0] == (size_t)-1) { /* No matching result (or no query). */
		colorize_no_match(tty, selected == 0 ? NULL : sel_color,
			selected == 0 ? choice : dchoice, pointer);
	} else { /* We have matches (and a query). */
		colorize_match(state, positions, dchoice, selected == 0
			? orig_color : sel_color, pointer, selected);
	}

	tty_setnormal(tty);
}

static char *
build_pointer(const int current, const int selected, const options_t *options)
{
	static char ptr_cur_sel[MAX_POINTER_LEN] = "";
	static char ptr_cur_nosel[MAX_POINTER_LEN];
	static char ptr_nocur_sel[MAX_POINTER_LEN];
	static char ptr_nocur_nosel[MAX_POINTER_LEN];

	/* Let's construct the pointer string only once */
	if (!*ptr_cur_sel) {
		/* Current (hovered) and selected */
		snprintf(ptr_cur_sel, sizeof(ptr_cur_sel), "%*s%s%s%s%s%s%s",
			options->pad, "", colors[SEL_BG_COLOR], colors[POINTER_COLOR],
			options->pointer, colors[MARKER_COLOR], options->marker, RESET_ATTR);

		/* Current (hovered) and not selected */
		snprintf(ptr_cur_nosel, sizeof(ptr_cur_nosel), "%*s%s%s%s%s%s%s",
			options->pad, "", colors[SEL_BG_COLOR], colors[POINTER_COLOR],
			options->pointer, colors[MARKER_COLOR], " ", RESET_ATTR);

		/* Not current (not hovered) and selected */
		snprintf(ptr_nocur_sel, sizeof(ptr_nocur_sel), "%*s%s%s%s%s%s",
			options->pad, "", colors[POINTER_COLOR], " ",
			colors[MARKER_COLOR], options->marker, RESET_ATTR);

		/* Not current (not hovered) and not selected */
		snprintf(ptr_nocur_nosel, sizeof(ptr_nocur_nosel), "%*s%s%s%s%s%s",
			options->pad, "", colors[POINTER_COLOR], " ",
			colors[MARKER_COLOR], " ", RESET_ATTR);
	}

	if (current == 1)
		return selected == 1 ? ptr_cur_sel : ptr_cur_nosel;

	return selected == 1 ? ptr_nocur_sel : ptr_nocur_nosel;
}

static void
draw(tty_interface_t *state)
{
	if (state->redraw == 0) {
		state->redraw = 1;
		return;
	}

	tty_t *tty = state->tty;
	choices_t *choices = state->choices;
	options_t *options = state->options;

	const unsigned int num_lines = options->num_lines;
	const size_t current_selection = choices->selection;
	size_t start = 0;

	if (current_selection + options->scrolloff >= num_lines) {
		start = current_selection + options->scrolloff - num_lines + 1;
		const size_t available = choices_available(choices);
		if (start + num_lines >= available && available > 0)
			start = available - num_lines;
	}

	const int options_multi = options->multi;
	const int options_pad = options->pad;
	const int options_reverse = options->reverse;
	const int options_show_info = options->show_info;

	tty_hide_cursor(tty);
	tty_setnowrap(tty);

	if (options_reverse == 0) {
		/* Set column, print prompt, and clear line. */
		tty_printf(tty, "\x1b[%dG%s%s%s", options_pad + 1,
			options->prompt, state->search, CLEAR_LINE);

		if (options_show_info == 1) {
			tty_printf(tty, "\n[%zu/%zu]%s", choices->available,
				choices->size, CLEAR_LINE);
		}
	} else if (num_lines + 1 + options_show_info >= tty->maxheight) {
		/* Fix the phantom lines issue present in some terminals. */
		tty_fputs(tty, "\x1b[A\r\x1b[K");
	}

	/* Print matches */
	const char *clear_line = options_reverse == 0 ? "\n"CLEAR_LINE : CLEAR_LINE;
	for (size_t i = start; i < start + num_lines; i++) {
		tty_fputs(tty, clear_line);

		const char *choice = choices_get(choices, i);
		if (choice) {
			const int selected = (options_multi == 1 && is_selected(choice));
			const int current = (i == choices->selection);
			const char *pointer = build_pointer(current, selected, options);

			draw_match(state, choice, current, pointer);
		}

		if (options_reverse == 1)
			tty_putc(tty, '\n');
	}

	if (options_reverse == 0 && (num_lines + options_show_info) > 0)
		tty_moveup(tty, num_lines + options_show_info);

	if (options_reverse == 1 && options_show_info == 1)
		tty_printf(tty, "\x1b[%dG[%zu/%zu]%s\n", options_pad + 1,
			choices->available, choices->size, CLEAR_LINE);

	/* Let's draw the prompt */
	static size_t prompt_len = (size_t)-1;
	if (prompt_len == (size_t)-1)
		prompt_len = wc_xstrlen(options->prompt);

	const size_t cursor_position =
		get_cursor_position(prompt_len + options_pad + 1, state);

	tty_printf(tty, "%s\x1b[%dG%s%s%s%s\x1b[%zuG", CLEAR_LINE,
		options_pad + 1, colors[PROMPT_COLOR], options->prompt,
		RESET_ATTR, state->search, cursor_position);

	tty_setwrap(tty);
	tty_unhide_cursor(tty);
	tty_flush(tty);
}

static void
update_search(tty_interface_t *state)
{
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
			tty_printf(state->tty, "\x1b[?25l\x1b[%dA\n",
				state->options->num_lines + 1 + state->options->show_info);
		}
		draw(state);
	}
}

/* Select the currently highighted/hovered entry if not already selected.
 * Otherwise, remove it from the selections list. */
static void
action_select(tty_interface_t *state)
{
	const char *p = choices_get(state->choices, state->choices->selection);
	if (!p)
		return;

	if (is_selected(p) == 1) {
		deselect_entry(p);
		return;
	}

	save_selection(p);
}

static void
action_emit(tty_interface_t *state)
{
	update_state(state);

	if (state->options->reverse == 1) {
		/* Move the cursor up and clear. */
		tty_printf(state->tty, "\x1b[%dA\x1b[J",
			state->options->num_lines + state->options->show_info);
	}

	if (state->options->multi == 1 && seln > 0) {
		clear(state);
		tty_close(state->tty);

		print_selections(state);
		free_selections(state);
		state->exit = EXIT_SUCCESS;
		return;
	}

	/* Reset the tty as close as possible to the previous state. */
	clear(state);

	/* ttyout should be flushed before outputting on stdout. */
	tty_close(state->tty);

	const char *selection =
		choices_get(state->choices, state->choices->selection);

	if (selection) { /* Output the selected result */
		const char *p = (*selection == KEY_ESC || strchr(selection, KEY_ESC))
			? decolor_name(selection, NULL) : selection;
		printf("%s%c", p, state->options->print_null ? '\0' : '\n');
		state->exit = EXIT_SUCCESS;
	} else {
		state->exit = EXIT_FAILURE;
	}
}

static void
action_backspace(tty_interface_t *state)
{
	if (state->cursor == 0) {
		state->redraw = 0;
		return;
	}

	const size_t length = strlen(state->search);
	const size_t original_cursor = state->cursor;

	do {
		state->cursor--;
	} while (state->cursor > 0 && !is_boundary(state->search[state->cursor]));

	memmove(&state->search[state->cursor], &state->search[original_cursor],
		length - original_cursor + 1);
}

static void
action_del(tty_interface_t *state)
{
	size_t length = strlen(state->search);
	if (state->cursor >= length) {
		state->redraw = 0;
		return;
	}

	size_t cursor = state->cursor;

	do {
		cursor++;
	} while (state->search[cursor] && !is_boundary(state->search[cursor]));

	memmove(&state->search[state->cursor], &state->search[cursor],
		length - cursor + 1);
}

static void
action_del_word(tty_interface_t *state)
{
	if (!*state->search) {
		state->redraw = 0;
		return;
	}

	const size_t original_cursor = state->cursor;
	size_t cursor = state->cursor;

	while (cursor && isspace(state->search[cursor - 1]))
		cursor--;

	while (cursor && !isspace(state->search[cursor - 1]))
		cursor--;

	memmove(&state->search[cursor], &state->search[original_cursor],
		strlen(state->search) - original_cursor + 1);
	state->cursor = cursor;
}

static void
action_del_all(tty_interface_t *state)
{
	if (!*state->search) {
		state->redraw = 0;
		return;
	}

	memmove(state->search, &state->search[state->cursor],
		strlen(state->search) - state->cursor + 1);
	state->cursor = 0;
}

static void
action_ignore(tty_interface_t *state)
{
	(void)state;
}

static void
action_prev(tty_interface_t *state)
{
	if (state->options->cycle == 0 && state->choices->selection == 0) {
		if (state->redraw != FORCE_REDRAW)
			state->redraw = 0;
		return;
	}

	update_state(state);
	choices_prev(state->choices);
}

static void
action_next(tty_interface_t *state)
{
	if (state->options->cycle == 0
	&& state->choices->selection + 1 >= state->choices->available) {
		if (state->redraw != FORCE_REDRAW)
			state->redraw = 0;
		return;
	}

	update_state(state);
	choices_next(state->choices);
}

static void
action_exit(tty_interface_t *state)
{
	if (state->options->reverse == 1) {
		/* Move the cursor up and clear. */
		tty_printf(state->tty, "\x1b[%dA\x1b[J",
			state->options->num_lines + state->options->show_info);
	}

	clear(state);
	tty_close(state->tty);

	state->exit = SIG_INTERRUPT;
}

static void
action_ctrl_d(tty_interface_t *state)
{
	if (*state->search)
		action_del(state);
	else
		action_exit(state);
}

static void
action_left(tty_interface_t *state)
{
	if (state->options->left_aborts == 1) {
		action_exit(state);
		return;
	}

	if (state->cursor > 0) {
		state->cursor--;
		while (state->cursor > 0 && !is_boundary(state->search[state->cursor]))
			state->cursor--;
	} else {
		state->redraw = 0;
	}
}

static void
action_right(tty_interface_t *state)
{
	if (state->options->right_accepts == 1) {
		action_emit(state);
		return;
	}

	if (state->cursor < strlen(state->search)) {
		state->cursor++;
		while (!is_boundary(state->search[state->cursor]))
			state->cursor++;
	} else {
		state->redraw = 0;
	}
}

static void
action_beginning(tty_interface_t *state)
{
	if (!*state->search || state->cursor == 0) {
		state->redraw = 0;
		return;
	}

	state->cursor = 0;
}

static void
action_end(tty_interface_t *state)
{
	size_t len = 0;
	if (!*state->search || (len = strlen(state->search)) == state->cursor) {
		state->redraw = 0;
		return;
	}

	state->cursor = len;
}

static void
action_pageup(tty_interface_t *state)
{
	const unsigned int num_lines = state->options->num_lines;
	const size_t selection = state->choices->selection;
	const int cycle = state->options->cycle;

	if (cycle == 0 && selection == 0) {
		state->redraw = 0;
		return;
	}

	update_state(state);

	for (size_t i = 0; i < num_lines && selection > 0; i++) {
		if (cycle == 0 && state->choices->selection == 0)
			break;

		choices_prev(state->choices);
	}
}

static void
action_pagedown(tty_interface_t *state)
{
	const unsigned int num_lines = state->options->num_lines;
	const size_t selection = state->choices->selection;
	const size_t available = state->choices->available;
	const int cycle = state->options->cycle;

	if (cycle == 0 && selection + 1 >= available) {
		state->redraw = 0;
		return;
	}

	update_state(state);

	for (size_t i = 0; i < num_lines && selection < available - 1; i++) {
		if (cycle == 0 && state->choices->selection + 1 >= available)
			break;

		choices_next(state->choices);
	}
}

static void
action_shift_tab(tty_interface_t *state)
{
	if (state->options->multi == 1) {
		action_select(state);
		/* We want to redraw even if at the top of the list. */
		state->redraw = FORCE_REDRAW;
		action_prev(state);
		state->redraw = 1;
	}
}

static void
action_tab(tty_interface_t *state)
{
	if (state->options->multi == 1) {
		action_select(state);
		/* We want to redraw even if at the bottom of the list. */
		state->redraw = FORCE_REDRAW;
		action_next(state);
		state->redraw = 1;
		return;
	}

	if (state->options->tab_accepts == 1)
		action_emit(state);
}

static void
append_search(tty_interface_t *state, const char ch)
{
	char *search = state->search;
	const size_t search_size = strlen(search);
	if (search_size < SEARCH_SIZE_MAX) {
		memmove(&search[state->cursor + 1], &search[state->cursor],
			search_size - state->cursor + 1);
		search[state->cursor] = ch;
		state->cursor++;
	}
}

void
tty_interface_init(tty_interface_t *state, tty_t *tty, choices_t *choices,
	options_t *options)
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

	if (options->init_search) {
		strncpy(state->search, options->init_search, SEARCH_SIZE_MAX);
		state->cursor = strlen(state->search);
	}

	update_search(state);
}

typedef struct {
	const char *key;
	const size_t keylen;
	void (*action)(tty_interface_t *);
} keybinding_t;

#define KEY_CTRL(key) ((const char[]){((key) - ('@')), '\0'})

static const keybinding_t keybindings[] = {
	{"\x1b[B", 3, action_next},           /* DOWN */
	{"\x1bOB", 3, action_next},           /* DOWN */
	{"\x1b[A", 3, action_prev},           /* UP */
	{"\x1bOA", 3, action_prev},           /* UP */
	{"\x1b", 1, action_exit},             /* ESC */
	{"\x7f", 1, action_backspace},	      /* Backspace */
	{KEY_CTRL('H'), 1, action_backspace}, /* Backspace (C-H) */
	{KEY_CTRL('W'), 1, action_del_word},  /* C-W */
	{KEY_CTRL('U'), 1, action_del_all},   /* C-U */
	{KEY_CTRL('I'), 1, action_tab},       /* TAB (C-I ) */
	{KEY_CTRL('C'), 1, action_exit},      /* C-C */
	{KEY_CTRL('D'), 1, action_ctrl_d},    /* C-D */
	{KEY_CTRL('G'), 1, action_exit},      /* C-G */
	{KEY_CTRL('M'), 1, action_emit},      /* CR */
	{KEY_CTRL('P'), 1, action_prev},      /* C-P */
	{KEY_CTRL('N'), 1, action_next},      /* C-N */
	{KEY_CTRL('K'), 1, action_prev},      /* C-K */
	{KEY_CTRL('J'), 1, action_next},      /* C-J */
	{KEY_CTRL('A'), 1, action_beginning}, /* C-A */
	{KEY_CTRL('E'), 1, action_end},   	  /* C-E */
	{"\x1b[3~", 4, action_del},           /* DEL */
	{"\x1bOD", 3, action_left},           /* LEFT */
	{"\x1b[D", 3, action_left},           /* LEFT */
	{"\x1bOC", 3, action_right},          /* RIGHT */
	{"\x1b[C", 3, action_right},          /* RIGHT */
	{"\x1b[1~", 4, action_beginning},     /* HOME */
	{"\x1b[7~", 4, action_beginning},     /* HOME: rxvt */
	{"\x1b[H", 3, action_beginning},      /* HOME */
	{"\x1bOH", 3, action_beginning},      /* HOME: VTE */
	{"\x1b[8~", 4, action_end},           /* END: rxvt */
	{"\x1b[4~", 4, action_end},           /* END */
	{"\x1bOF", 3, action_end},            /* END: VTE*/
	{"\x1b[F", 3, action_end},            /* END */
	{"\x1b[5~", 4, action_pageup},
	{"\x1b[6~", 4, action_pagedown},
	{"\x1b[200~", 6, action_ignore},
	{"\x1b[201~", 6, action_ignore},
	{"\x1b[Z", 3, action_shift_tab},      /* Shift-TAB */
	{NULL, 0, NULL}
};
#undef KEY_CTRL

static void
handle_input(tty_interface_t *state, const char *s,
	const int handle_ambiguous_key)
{
	state->ambiguous_key_pending = 0;

	char *input = state->input;
	size_t input_len = *state->input ? strlen(state->input) : 0;
	strncat(state->input, s, sizeof(state->input) - input_len - 1);
	/* S is either a single byte or empty. If a single byte, increase INPUT_LEN. */
	input_len += (*s != '\0');

	/* Figure out if we have completed a keybinding and whether we're in the
	 * middle of one (both can happen, because of Esc). */
	int found_keybinding = -1;
	int in_middle = 0;

	for (int i = 0; keybindings[i].key; i++) {
		if (*input != *keybindings[i].key || (input_len > 1
		&& input[1] != keybindings[i].key[1]))
			continue;

		if (keybindings[i].keylen == input_len
		&& strcmp(input, keybindings[i].key) == 0)
			found_keybinding = i;
		else if (strncmp(input, keybindings[i].key, input_len) == 0)
			in_middle = 1;
	}

	/* If we have an unambiguous keybinding, run it.  */
	if (found_keybinding != -1 && (!in_middle || handle_ambiguous_key)) {
		keybindings[found_keybinding].action(state);
		*input = '\0';
		return;
	}

	/* Wait for more if we are in the middle of a keybinding. */
	if (in_middle == 1) {
		state->ambiguous_key_pending = 1;
		return;
	}

	/* No matching keybinding, add to search.
	 * Exclude input starting with non-printing char, mostly keybindings,
	 * e.g. INSERT, etc. */
	if (isprint_unicode(*input)) {
		for (int i = 0; input[i]; i++) {
			if (isprint_unicode(input[i]))
				append_search(state, input[i]);
		}
	}

	/* We have processed the input, so clear it. */
	*input = '\0';
}

int
tty_interface_run(tty_interface_t *state)
{
	if (state->options->no_color == 0)
		set_colors(state);
	if (state->options->auto_lines == 1) {
		state->options->num_lines =
			tty_getheight(state->tty) - 1 - state->options->show_info;
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
				tty_printf(state->tty, "\x1b[?25l\x1b[%dA\n",
					state->options->num_lines + 1 + state->options->show_info);
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
