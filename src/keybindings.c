/* keybindings.c */

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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "colors.h"
#include "keybindings.h"
#include "selections.h"

static int
isprint_unicode(const char c)
{
	return (isprint(c) || c & (1 << 7));
}

static void
set_cursor_for_clear(const tty_interface_t *state)
{
	if (state->options->reverse == 1) {
		if (state->options->clear == 1) {
			/* Move the cursor up. */
			tty_printf(state->tty, "\x1b[%zuA",
				state->options->num_lines + (size_t)state->options->show_info);
		} else {
			tty_putc(state->tty, '\n');
		}
	} else if (state->options->clear == 0) {
		/* Move the cursor down and print a new line. */
		tty_printf(state->tty, "\x1b[%zuB\n",
			state->options->num_lines + (size_t)state->options->show_info + 1);
	}
}

/* Reset the tty as close as possible to the previous state. */
static void
clear(const tty_interface_t *state)
{
	set_cursor_for_clear(state);

	if (state->options->clear == 0) {
		tty_flush(state->tty);
		return;
	}

	const tty_t *tty = state->tty;
	const size_t num_lines = state->options->num_lines;
	const int show_info = state->options->show_info;

	tty_setcol(tty, state->options->pad);
	size_t line = 0;
	while (line++ < num_lines + (show_info ? 1 : 0))
		tty_newline(tty);

	tty_clearline(tty);
	if (num_lines > 0)
		tty_moveup(tty, (int)line - 1);

	tty_flush(tty);
}

/* Select the currently highighted/hovered entry if not already selected.
 * Otherwise, remove it from the selections list. */
static void
action_select(tty_interface_t *state)
{
	const char *choice = choices_get(state->choices, state->choices->selection);
	if (!choice)
		return;

	if (is_selected(choice) == 1) {
		deselect_entry(choice, state);
		return;
	}

	save_selection(choice, state->selection);
}

static void
action_exit(tty_interface_t *state)
{
	clear(state);
	tty_close(state->tty);
	state->exit = SIG_INTERRUPT;
}

static void
action_emit(tty_interface_t *state)
{
	clear(state);
	/* ttyout should be flushed before outputting on stdout. */
	tty_close(state->tty);

	if (state->options->multi == 1 && state->selection->selected > 0) {
		print_selections(state);
		free_selections(state);
		state->exit = EXIT_SUCCESS;
		return;
	}

	if (state->selection->size > 0)
		free_selections(state);

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

	if (state->options->cycle == 1)
		choices_prev(state->choices);
	else
		state->choices->selection--;
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

	if (state->options->cycle == 1)
		choices_next(state->choices);
	else
		state->choices->selection++;
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
action_first(tty_interface_t *state)
{
	if (state->choices->selection == 0) {
		state->redraw = 0;
		return;
	}

	state->choices->selection = 0;
}

static void
action_last(tty_interface_t *state)
{
	if (state->choices->selection == state->choices->available - 1) {
		state->redraw = 0;
		return;
	}

	state->choices->selection = state->choices->available - 1;
}

static void
action_pageup(tty_interface_t *state)
{
	const size_t num_lines = state->options->num_lines;
	const size_t selection = state->choices->selection;
	const int cycle = state->options->cycle;

	if (cycle == 0 && selection == 0) {
		state->redraw = 0;
		return;
	}

	for (size_t i = 0; i < num_lines; i++) {
		if (cycle == 0 && state->choices->selection == 0)
			break;

		choices_prev(state->choices);
	}
}

static void
action_pagedown(tty_interface_t *state)
{
	const size_t num_lines = state->options->num_lines;
	const size_t selection = state->choices->selection;
	const size_t available = state->choices->available;
	const int cycle = state->options->cycle;

	if (cycle == 0 && selection + 1 >= available) {
		state->redraw = 0;
		return;
	}

	for (size_t i = 0; i < num_lines && selection <= available - 1; i++) {
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
	{"\x7f", 1, action_backspace},	      /* Backspace (DEL) */
	{KEY_CTRL('H'), 1, action_backspace}, /* Backspace (C-H) */
	{KEY_CTRL('W'), 1, action_del_word},  /* Ctrl-W */
	{KEY_CTRL('U'), 1, action_del_all},   /* Ctrl-U */
	{KEY_CTRL('I'), 1, action_tab},       /* TAB (Ctrl-I ) */
	{KEY_CTRL('C'), 1, action_exit},      /* Ctrl-C */
	{KEY_CTRL('D'), 1, action_ctrl_d},    /* Ctrl-D */
	{KEY_CTRL('G'), 1, action_exit},      /* Ctrl-G */
	{KEY_CTRL('M'), 1, action_emit},      /* CR (Enter) */
	{KEY_CTRL('P'), 1, action_prev},      /* Ctrl-P */
	{KEY_CTRL('N'), 1, action_next},      /* Ctrl-N */
	{KEY_CTRL('K'), 1, action_prev},      /* Ctrl-K */
	{KEY_CTRL('J'), 1, action_next},      /* Ctrl-J */
	{KEY_CTRL('A'), 1, action_beginning}, /* Ctrl-A */
	{KEY_CTRL('E'), 1, action_end},   	  /* Ctrl-E */
	{KEY_CTRL('F'), 1, action_right},     /* Ctrl-F */
	{KEY_CTRL('B'), 1, action_left},      /* Ctrl-B */
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
	{"\x1b[5~", 4, action_pageup},        /* PgUp */
	{"\x1b[6~", 4, action_pagedown},      /* PgDn */
	{"\x1b[200~", 6, action_ignore},      /* Begin bracketed paste */
	{"\x1b[201~", 6, action_ignore},      /* End bracketed paste */
	{"\x1b[Z", 3, action_shift_tab},      /* Shift-TAB */
	{"\x1b[1;5H", 6, action_first},       /* Ctrl-Home */
	{"\x1b[5;5~", 6, action_first},       /* Ctrl-PgUp */
	{"\x1b[7^", 4, action_first},         /* Ctrl-Home (rxvt) */
	{"\x1b[5^", 4, action_first},         /* Ctrl-PgUp (rxvt) */
	{"\x1b[1;5F", 6, action_last},        /* Ctrl-End */
	{"\x1b[6;5~", 6, action_last},        /* Ctrl-PgDn */
	{"\x1b[6^", 4, action_last},          /* Ctrl-PgDn (rxvt) */
	{"\x1b[8^", 4, action_last},          /* Ctrl-End (rxvt) */
	{NULL, 0, NULL}
};
#undef KEY_CTRL

/* This function is called repeatedly (from tty_interface_run()) until we get
 * a complete keybinding.
 * If this keybinding is associated to a function, run it. Otherwise,
 * append the input to the query string in the prompt. */
void
handle_input(tty_interface_t *state, const char *s,
	const int handle_ambiguous_key)
{
	state->ambiguous_key_pending = 0;

	char *input = state->input;
	size_t input_len = *input ? strlen(input) : 0;

	if (!s || input_len >= sizeof(state->input) - 1) {
		*input = '\0';
		return;
	}

	/* Append the current input byte. */
	input[input_len] = *s;
	input[input_len + 1] = '\0';
	/* S is either a single byte or empty. If a single byte,
	 * increase INPUT_LEN. */
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
	 * Exclude input starting with non-printing characters, mostly keybindings,
	 * e.g. INSERT, function keys, etc. */
	if (isprint_unicode(*input)) {
		for (size_t i = 0; input[i]; i++) {
			if (isprint_unicode(input[i]))
				append_search(state, input[i]);
		}
	}

	/* We have processed the input: clear it. */
	*input = '\0';
}
