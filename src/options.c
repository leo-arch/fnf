/* options.c */

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

#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"
#include "config.h"

static const char *usage_str =
    ""
    "Usage: fnf [OPTION]...\n"
    " -l, --lines=LINES        Specify how many lines of results to show (default 10)\n"
    " -m, --multi              Enable multi-selection\n"
    " -p, --prompt=PROMPT      Input prompt (default \"> \")\n"
    " -P, --pad=NUM            Left pad the list of matches NUM places (default 0)\n"
    " -q, --query=QUERY        Use QUERY as the initial search string\n"
    " -e, --show-matches=QUERY Output the sorted matches of QUERY\n"
    " -t, --tty=TTY            Specify file to use as TTY device (default /dev/tty)\n"
    " -s, --show-scores        Show the scores of each match\n"
    " -0, --read-null          Read input delimited by ASCII NUL characters\n"
    " -j, --workers=NUM        Use NUM workers for searching. (default is # of CPUs)\n"
    " -i, --show-info          Show selection info line\n"
    " -h, --help               Display this help and exit\n"
    " -v, --version            Output version information and exit\n"
    "     --pointer=STRING     Pointer to highlighted match (default \">\")\n"
    "     --marker=STRING      Multi-select marker (default \"*\")\n"
    "     --cycle              Enable cyclic scrolling\n"
    "     --tab-accepts        TAB accepts\n"
    "     --right-accepts      Right arrow key accepts\n"
    "     --left-aborts        Left arrow key aborts\n"
    "     --reverse            Display from top, prompt at bottom\n"
    "     --no-unicode         Do not use Unicode decorations\n"
    "     --no-color           Run colorless\n";

static void
usage(const char *argv0)
{
	fprintf(stderr, usage_str, argv0);
}

static struct option longopts[] = {
	{"show-matches", required_argument, NULL, 'e'},
	{"query", required_argument, NULL, 'q'},
	{"lines", required_argument, NULL, 'l'},
	{"tty", required_argument, NULL, 't'},
	{"prompt", required_argument, NULL, 'p'},
	{"show-scores", no_argument, NULL, 's'},
	{"read-null", no_argument, NULL, '0'},
	{"version", no_argument, NULL, 'v'},
	{"benchmark", optional_argument, NULL, 'b'},
	{"workers", required_argument, NULL, 'j'},
	{"show-info", no_argument, NULL, 'i'},
	{"help", no_argument, NULL, 'h'},
	{"pad", required_argument, NULL, 'P'},
	{"multi", no_argument, NULL, 'm'},
	{"pointer", required_argument, NULL, 1},
	{"marker", required_argument, NULL, 2},
	{"cycle", no_argument, NULL, 3},
	{"tab-accepts", no_argument, NULL, 4},
	{"right-accepts", no_argument, NULL, 5},
	{"left-aborts", no_argument, NULL, 6},
	{"no-color", no_argument, NULL, 7},
	{"reverse", no_argument, NULL, 8},
	{"no-unicode", no_argument, NULL, 9},
	{NULL, 0, NULL, 0}
};

void
options_init(options_t *options)
{
	/* Set defaults */
	options->benchmark       = DEFAULT_BENCHMARK;
	options->filter          = DEFAULT_FILTER;
	options->init_search     = DEFAULT_INIT_SEARCH;
	options->show_scores     = DEFAULT_SCORES;
	options->scrolloff       = DEFAULT_SCROLLOFF;
	options->tty_filename    = DEFAULT_TTY;
	options->num_lines       = DEFAULT_NUM_LINES;
	options->auto_lines      = DEFAULT_AUTO_LINES;
	options->prompt          = DEFAULT_PROMPT;
	options->workers         = DEFAULT_WORKERS;
	options->input_delimiter = DEFAULT_DELIMITER;
	options->show_info       = DEFAULT_SHOW_INFO;
	options->pad             = DEFAULT_PAD;
	options->multi           = DEFAULT_MULTI;
	options->pointer         = DEFAULT_POINTER;
	options->marker          = DEFAULT_MARKER;
	options->cycle           = DEFAULT_CYCLE;
	options->tab_accepts     = DEFAULT_TAB_ACCEPTS;
	options->right_accepts   = DEFAULT_RIGHT_ACCEPTS;
	options->left_aborts     = DEFAULT_LEFT_ABORTS;
	options->no_color        = DEFAULT_NO_COLOR;
	options->reverse         = DEFAULT_REVERSE;
	options->unicode         = DEFAULT_UNICODE;
}

void
options_parse(options_t *options, int argc, char *argv[])
{
	options_init(options);
	int pointer_set = 0;
	int marker_set = 0;

	int c;
	while ((c = getopt_long(argc, argv, "mvhs0e:q:l:t:p:P:j:i", longopts, NULL)) != -1) {
		switch (c) {
		case 'v':
			printf("%s\n", VERSION);
			exit(EXIT_SUCCESS);
		case 's': options->show_scores = 1;	break;
		case '0': options->input_delimiter = '\0'; break;
		case 'm': options->multi = 1; break;
		case 'q': options->init_search = optarg; break;
		case 'e': options->filter = optarg; break;
		case 'b':
			if (optarg) {
				if (sscanf(optarg, "%d", &options->benchmark) != 1) {
					usage(argv[0]);
					exit(EXIT_FAILURE);
				}
			} else {
				options->benchmark = 100;
			}
			break;
		case 't': options->tty_filename = optarg; break;
		case 'p': options->prompt = optarg; break;
		case 'P':
			if (optarg && *optarg && *optarg >= '0' && *optarg <= '9')
				options->pad = atoi(optarg);
			break;
		case 'j':
			if (sscanf(optarg, "%u", &options->workers) != 1) {
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			break;
		case 'l': {
			if (!optarg)
				break;
			int l;
			if (!strcmp(optarg, "max")) {
				l = INT_MAX;
			} else if (!strcmp(optarg, "auto")) {
				l = 0;
				options->auto_lines = 1;
			} else if (sscanf(optarg, "%d", &l) != 1 || l < 2) {
				fprintf(stderr, "Invalid format for --lines: %s\n", optarg);
				fprintf(stderr, "Must be integer in range 2..\n");
				exit(EXIT_FAILURE);
			}
			options->num_lines = l;
		} break;
		case 'i': options->show_info = 1; break;
		case 1:
			if (optarg && *optarg) {
				pointer_set = 1;
				options->pointer = optarg;
			}
			break;
		case 2:
			if (optarg && *optarg) {
				marker_set = 1;
				options->marker = optarg;
			}
			break;
		case 3: options->cycle = 1;	break;
		case 4:	options->tab_accepts = 1; break;
		case 5: options->right_accepts = 1; break;
		case 6:	options->left_aborts = 1; break;
		case 7:	options->no_color = 1; break;
		case 8:	options->reverse = 1; break;
		case 9: options->unicode = 0; break;

		case 'h': /* fallthrough */
		default:
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		}
	}

	if (optind != argc) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (options->unicode != 0) {
		if (pointer_set == 0)
			options->pointer = DEFAULT_POINTER_UNICODE;
		if (marker_set == 0)
			options->marker = DEFAULT_MARKER_UNICODE;
	}
}
