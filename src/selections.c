/* selections.c */

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

#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "selections.h"

/* Struct to store selected/marked entries */
struct selections_t {
	char *name;
	size_t namelen;
};

static struct selections_t *selections = (struct selections_t *)NULL;

/* SELN is the current size of the selections array, while SEL_COUNTER
 * is the current number of actually selected entries (i.e.
 * seln - deselected_entries, since deselected entries are nullyfied). */
size_t seln = 0;
static size_t sel_counter = 0;

/* Search for the string P in the selections array. If found, return 1,
 * otherwise zero. */
int
is_selected(const char *name)
{
	if (!name || !*name || sel_counter == 0)
		return 0;

	const size_t len = strlen(name);
	size_t i;
	for (i = 0; selections[i].name; i++) {
		if (*selections[i].name == *name && selections[i].namelen == len
		&& strcmp(selections[i].name, name) == 0)
			return 1;
	}

	return 0;
}

/* Remote the entry NAME from the selections array by setting the first
 * byte of the corresponding array entry to NUL */
void
deselect_entry(const char *name)
{
	if (!name || !*name || sel_counter == 0)
		return;

	const size_t len = strlen(name);
	size_t i;
	for (i = 0; selections[i].name; i++) {
		if (*selections[i].name != *name || selections[i].namelen != len
		|| strcmp(selections[i].name, name) != 0)
			continue;
		*selections[i].name = '\0';
		sel_counter--;
		break;
	}
}

/* Save the string P into the selections array. */
void
save_selection(const char *name)
{
	selections = (struct selections_t *)realloc(
		selections, (seln + 2) * sizeof(struct selections_t));
	if (!selections)
		return;

	const size_t len = strlen(name);
	selections[seln].name = (char *)malloc((len + 1) * sizeof(char));
	if (!selections[seln].name)
		return;

	strcpy(selections[seln].name, name);
	selections[seln].namelen = len;
	seln++;
	sel_counter++;
	selections[seln].name = (char *)NULL;
	selections[seln].namelen = 0;
}

/* Print the list of selected/marked entries to STDOUT. */
void
print_selections(tty_interface_t *state)
{
	if (sel_counter == 0 || state->options->multi == 0)
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
	if (state->options->multi == 0 || seln == 0 || !selections)
		return;

	for (size_t i = 0; selections[i].name; i++)
		free(selections[i].name);
	free(selections);
	selections = (struct selections_t *)NULL;
}
