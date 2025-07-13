/* selections.c */

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
#include <string.h>

#include "colors.h"
#include "tty_interface.h"

/* Array to store selected/marked entries */
static char **selections = (char **)NULL;

/* SELN is the current size of the selections array, while SEL_COUNTER
 * is the current number of actually selected entries. */
size_t seln = 0;
static size_t sel_counter = 0;

/* Search for the string P in the selections array. If found, return 1,
 * otherwise zero. */
int
is_selected(const char *p)
{
	if (!p || !*p || sel_counter == 0)
		return 0;

	size_t i;
	for (i = 0; selections[i]; i++) {
		if (*selections[i] == *p && strcmp(selections[i], p) == 0)
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

	size_t i;
	for (i = 0; selections[i]; i++) {
		if (*selections[i] != *name || strcmp(selections[i], name) != 0)
			continue;
		*selections[i] = '\0';
		sel_counter--;
		break;
	}
}

/* Save the string P into the selections array. */
void
save_selection(const char *p)
{
	selections = (char **)realloc(selections, (seln + 2) * sizeof(char *));
	if (!selections)
		return;

	selections[seln] = (char *)malloc((strlen(p) + 1) * sizeof(char));
	if (!selections[seln])
		return;

	strcpy(selections[seln], p);
	seln++;
	sel_counter++;
	selections[seln] = (char *)NULL;
}

/* Print the list of selected/marked entries to STDOUT. */
void
print_selections(tty_interface_t *state)
{
	if (sel_counter == 0 || state->options->multi == 0)
		return;

	size_t i;
	for (i = 0; selections[i]; i++) {
		if (!*selections[i])
			continue;
		const char *name =
			(*selections[i] == KEY_ESC || strchr(selections[i], KEY_ESC))
			? decolor_name(selections[i]) : selections[i];
		puts(name);
	}

}

/* Free the selections array. */
void
free_selections(tty_interface_t *state)
{
	if (state->options->multi == 0 || seln == 0 || !selections)
		return;

	for (size_t i = 0; selections[i]; i++)
		free(selections[i]);
	free(selections);
	selections = (char **)NULL;
}
