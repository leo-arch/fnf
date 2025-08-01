/* fnf.c */

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

#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <unistd.h>
#include <locale.h> /* setlocale() */

#include "match.h"
#include "tty.h"
#include "choices.h"
#include "options.h"
#include "tty_interface.h"

#include "config.h"

int
main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");
	int ret = 0;

	options_t options;
	options_parse(&options, argc, argv);

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
			exit(EXIT_FAILURE);
		}

		tty_t tty;
		tty_init(&tty, options.tty_filename);

		if (!isatty(STDIN_FILENO))
			choices_fread(&choices, stdin, options.input_delimiter,
				options.max_items);

		if (options.num_lines > choices.size)
			options.num_lines = choices.size;

		if (options.scrolloff == -1) { /* --scroll-off=auto */
			const size_t tty_lines = tty_getheight(&tty);
			const size_t n = options.num_lines > tty_lines
				? tty_lines : options.num_lines;
			options.scrolloff = n >> 1; /* n / 2 */
		}

		const int num_lines_adjustment = 1 + options.show_info;

		if (options.num_lines + num_lines_adjustment > tty_getheight(&tty))
			options.num_lines = tty_getheight(&tty) - num_lines_adjustment;

		tty_interface_t tty_interface;
		tty_interface_init(&tty_interface, &tty, &choices, &options, &selection);
		ret = tty_interface_run(&tty_interface);
	}

	choices_destroy(&choices);

	return ret;
}
