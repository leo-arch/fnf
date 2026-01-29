/*
 * This file is part of fnf
 *
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2022-2026, L. Abramovich <leo.clifm@outlook.com>
*/

/* selections.c */

#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "selections.h"

/* Struct to store selected/marked entries. */
struct selections_t {
	char *name;
	size_t namelen;
};

static struct selections_t *selections = (struct selections_t *)NULL;

/* Search for the string NAME in the selections array. If found, return 1,
 * otherwise zero. */
int
is_selected(const char *name)
{
	if (!name || !*name || !selections)
		return 0;

	const size_t len = strlen(name);

	for (size_t i = 0; selections[i].name; i++) {
		if (*selections[i].name == *name && selections[i].namelen == len
		&& strcmp(selections[i].name, name) == 0)
			return 1;
	}

	return 0;
}

/* Remote the entry NAME from the selections array by setting the first
 * byte of the corresponding array entry to NUL */
void
deselect_entry(const char *name, tty_interface_t *state)
{
	if (!name || !*name || state->selection->selected == 0)
		return;

	const size_t len = strlen(name);
	for (size_t i = 0; selections[i].name; i++) {
		if (*selections[i].name != *name || selections[i].namelen != len
		|| strcmp(selections[i].name, name) != 0)
			continue;

		*selections[i].name = '\0';
		state->selection->selected--;
		break;
	}

	if (state->selection->selected == 0)
		free_selections(state);
}

/* Save the string NAME into the selections array. */
void
save_selection(const char *name, sel_t *selection)
{
	struct selections_t *tmp = (struct selections_t *)realloc(
		selections, (selection->size + 2) * sizeof(struct selections_t));
	if (!tmp) {
		for (size_t i = 0; selections[i].name; i++)
			free(selections[i].name);
		free(selections);
		fprintf(stderr, "Error: Cannot allocate memory\n");
		abort();
	}
	selections = tmp;

	const size_t len = strlen(name);
	selections[selection->size].name = (char *)malloc((len + 1) * sizeof(char));
	if (!selections[selection->size].name) {
		fprintf(stderr, "Error: Cannot allocate memory\n");
		abort();
	}

	selection->selected++;

	memcpy(selections[selection->size].name, name, len + 1);
	selections[selection->size].namelen = len;

	selection->size++;
	selections[selection->size].name = (char *)NULL;
	selections[selection->size].namelen = 0;
}

/* Print the list of selected/marked entries to STDOUT. */
void
print_selections(tty_interface_t *state)
{
	if (state->selection->selected == 0 || state->options->multi == 0)
		return;

	const char end_char = state->options->print_null ? '\0' : '\n';

	for (size_t i = 0; selections[i].name; i++) {
		if (!*selections[i].name)
			continue;

		const char *name = (*selections[i].name == KEY_ESC
			|| strchr(selections[i].name, KEY_ESC))
			? decolor_name(selections[i].name, NULL) : selections[i].name;

		printf("%s%c", name, end_char);
	}

}

/* Free the selections array. */
void
free_selections(tty_interface_t *state)
{
	if (state->options->multi == 0 || state->selection->size == 0
	|| !selections)
		return;

	for (size_t i = 0; selections[i].name; i++)
		free(selections[i].name);
	free(selections);
	selections = (struct selections_t *)NULL;

	state->selection->size = 0;
	state->selection->selected = 0;
}
