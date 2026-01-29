/*
 * This file is part of fnf
 *
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2014-2022 John Hawthorn <john.hawthorn@gmail.com>
 * SPDX-FileCopyrightText: 2022-2026, L. Abramovich <leo.clifm@outlook.com>
*/

/* fnf.c */

#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <unistd.h>
#include <locale.h> /* setlocale() */

#include "tty.h"
#include "choices.h"
#include "options.h"
#include "tty_interface.h"

#include "config.h"

static void
set_num_lines(options_t *options, const tty_t *tty, const size_t choices_size)
{
	if (options->num_lines == (size_t)-1)
		/* Unset: default to half of terminal height. */
		options->num_lines = tty->maxheight / 2;

	if (options->num_lines > choices_size)
		options->num_lines = choices_size;
}

int
main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");
	int ret = 0;

	options_t options;
	options_parse(&options, argc, argv);

	if (options.case_sens_mode != CASE_SMART)
		g_case_sensitive = (options.case_sens_mode == CASE_SENSITIVE);

	choices_t choices;
	choices_init(&choices, &options);

	sel_t selection = {0};

	if (options.filter) { /* --show-matches */
		choices_fread(&choices, stdin, options.input_delimiter,
			options.max_items);
		choices_search(&choices, options.filter, options.sort);
		for (size_t i = 0; i < choices_available(&choices); i++) {
			if (options.show_scores)
				printf("%f\t", choices_getscore(&choices, i));
			printf("%s\n", choices_get(&choices, i));
		}
	} else { /* Interactive */
		if (isatty(STDIN_FILENO)) {
			fputs("fnf: Expected piped input (e.g. 'ls | fnf')\n", stderr);
			choices_destroy(&choices);
			exit(EXIT_FAILURE);
		}

		tty_t tty;
		tty_init(&tty, options.tty_filename);

		if (!isatty(STDIN_FILENO))
			choices_fread(&choices, stdin, options.input_delimiter,
				options.max_items);

		set_num_lines(&options, &tty, choices.size);

		const size_t num_lines_adjustment = 1 + (size_t)options.show_info;

		if (options.num_lines + num_lines_adjustment > tty_getheight(&tty))
			options.num_lines = tty_getheight(&tty) - num_lines_adjustment;

		tty_interface_t tty_interface;
		tty_interface_init(&tty_interface, &tty, &choices, &options, &selection);
		ret = tty_interface_run(&tty_interface);
	}

	choices_destroy(&choices);

	return ret;
}
