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

#define OPT_POINTER       1
#define OPT_MARKER        2
#define OPT_TAB_ACCEPTS   3
#define OPT_RIGHT_ACCEPTS 4
#define OPT_LEFT_ABORTS   5
#define OPT_NO_COLOR      6
#define OPT_NO_UNICODE    7
#define OPT_COLOR         8
#define OPT_PRINT_NULL    9
#define OPT_SCROLLOFF     10
#define OPT_NO_SORT       11
#define OPT_NO_CLEAR      12
#define OPT_SEPARATOR     13
#define OPT_CASE          14

static const char *usage_str =
    ""
    "Usage: fnf [OPTION]...\n"
    " -0, --read-null          Read input delimited by ASCII NUL characters\n"
    " -c, --cycle              Enable cyclic scrolling\n"
    " -e, --show-matches=QUERY Display the sorted matches of QUERY and exit\n"
    " -h, --help               Display this help and exit\n"
    " -i, --show-info          Show selection info line\n"
    " -j, --workers=NUM        Use NUM workers for searching (default: # of CPUs)\n"
    " -l, --lines=NUM          Show only up to NUM results (default: half of terminal height)\n"
    " -m, --multi              Enable multi-selection\n"
    " -M, --max-items=NUM      Load only up to NUM items (default: unlimited)\n"
    " -p, --prompt=STR         Input prompt (default: \"> \")\n"
    " -P, --padding=NUM        Left pad the list of matches by NUM spaces (default: 0)\n"
    " -q, --query=QUERY        Use QUERY as the initial search string\n"
    " -r, --reverse            Display from top, prompt at bottom\n"
    " -s, --show-scores        Show the scores of each match\n"
    " -t, --tty=TTY            Specify the file to use as TTY device (default: /dev/tty)\n"
    " -v, --version            Output version information and exit\n"
    "     --case=MODE          Set case sensitivity mode [respect|ignore|smart] (default: smart)\n"
    "     --color=COLORSPEC    Set custom colors (consult the manpage)\n"
    "     --marker=STR         Multi-select marker (default: \"✔\" or \"*\")\n"
    "     --no-clear           Do not clear the interface on exit\n"
    "     --no-color           Disable colors\n"
    "     --no-sort            Do not sort the result\n"
    "     --no-unicode         Disable Unicode decorations\n"
    "     --pointer=STR        Pointer to highlighted match (default: \"▌\" or \">\")\n"
    "     --separator[=STR]    Print horizontal line after info\n"
    "     --print-null         Print ouput delimited by ASCII NUL characters\n"
    "     --right-accepts      Right arrow key accepts\n"
    "     --tab-accepts        TAB accepts\n"
    "     --left-aborts        Left arrow key aborts\n";

static void
usage(void)
{
	fprintf(stderr, "%s", usage_str);
}

static struct option longopts[] = {
	{"read-null", no_argument, NULL, '0'},
	{"cycle", no_argument, NULL, 'c'},
	{"show-matches", required_argument, NULL, 'e'},
	{"help", no_argument, NULL, 'h'},
	{"show-info", no_argument, NULL, 'i'},
	{"workers", required_argument, NULL, 'j'},
	{"lines", required_argument, NULL, 'l'},
	{"height", required_argument, NULL, 'l'},
	{"multi", no_argument, NULL, 'm'},
	{"max-items", required_argument, NULL, 'M'},
	{"prompt", required_argument, NULL, 'p'},
	{"padding", required_argument, NULL, 'P'},
	{"query", required_argument, NULL, 'q'},
	{"reverse", no_argument, NULL, 'r'},
	{"show-scores", no_argument, NULL, 's'},
	{"tty", required_argument, NULL, 't'},
	{"version", no_argument, NULL, 'v'},
	{"case", required_argument, NULL, OPT_CASE},
	{"color", required_argument, NULL, OPT_COLOR},
	{"left-aborts", no_argument, NULL, OPT_LEFT_ABORTS},
	{"marker", required_argument, NULL, OPT_MARKER},
	{"no-clear", no_argument, NULL, OPT_NO_CLEAR},
	{"no-color", no_argument, NULL, OPT_NO_COLOR},
	{"no-sort", no_argument, NULL, OPT_NO_SORT},
	{"no-unicode", no_argument, NULL, OPT_NO_UNICODE},
	{"pointer", required_argument, NULL, OPT_POINTER},
	{"print-null", no_argument, NULL, OPT_PRINT_NULL},
	{"right-accepts", no_argument, NULL, OPT_RIGHT_ACCEPTS},
	{"scroll-off", required_argument, NULL, OPT_SCROLLOFF},
	{"separator", optional_argument, NULL, OPT_SEPARATOR},
	{"tab-accepts", no_argument, NULL, OPT_TAB_ACCEPTS},
	{NULL, 0, NULL, 0}
};

/* Set options to default values. */
void
options_init(options_t *options)
{
	options->auto_lines      = DEFAULT_AUTO_LINES;
	options->case_sens_mode  = DEFAULT_CASE_SENSITIVITY_MODE;
	options->clear           = DEFAULT_CLEAR;
	options->color           = NULL; /* Unset*/
	options->cycle           = DEFAULT_CYCLE;
	options->filter          = DEFAULT_FILTER;
	options->init_search     = DEFAULT_INIT_SEARCH;
	options->input_delimiter = DEFAULT_DELIMITER;
	options->left_aborts     = DEFAULT_LEFT_ABORTS;
	options->marker          = DEFAULT_MARKER;
	options->max_items       = DEFAULT_MAX_ITEMS;
	options->multi           = DEFAULT_MULTI;
	options->no_color        = DEFAULT_NO_COLOR;
	options->num_lines       = (size_t)-1; /* Unset*/
	options->pad             = DEFAULT_PAD;
	options->pointer         = DEFAULT_POINTER;
	options->print_null      = DEFAULT_PRINT_NULL;
	options->prompt          = DEFAULT_PROMPT;
	options->reverse         = DEFAULT_REVERSE;
	options->right_accepts   = DEFAULT_RIGHT_ACCEPTS;
	options->show_info       = DEFAULT_SHOW_INFO;
	options->show_scores     = DEFAULT_SCORES;
	options->scrolloff       = DEFAULT_SCROLLOFF;
	options->separator       = NULL; /* Unset*/
	options->sort            = DEFAULT_SORT;
	options->tab_accepts     = DEFAULT_TAB_ACCEPTS;
	options->tty_filename    = DEFAULT_TTY;
	options->unicode         = DEFAULT_UNICODE;
	options->workers         = DEFAULT_WORKERS;
}

static void
set_padding(options_t *options, const char *optarg)
{
	if (optarg && *optarg >= '0' && *optarg <= '9')
		options->pad = atoi(optarg);
}

static void
set_max_items(options_t *options, const char *optarg)
{
	if (optarg && *optarg >= '0' && *optarg <= '9')
		options->max_items = atoi(optarg);
}

static void
set_workers(options_t *options, const char *optarg)
{
	if (sscanf(optarg, "%zu", &options->workers) != 1) {
		usage();
		exit(EXIT_FAILURE);
	}
}

static void
set_lines(options_t *options, const char *optarg)
{
	if (!optarg)
		return;

	int l = 0;
	if (*optarg == 'm' && strcmp(optarg, "max") == 0) {
		l = INT_MAX;
	} else if (*optarg == 'a' && strcmp(optarg, "auto") == 0) {
		options->auto_lines = 1;
	} else if (sscanf(optarg, "%d", &l) != 1 || l < 2) {
		fprintf(stderr, "Invalid format for --lines: %s\n", optarg);
		fprintf(stderr, "Must be an integer in the range 2..\n");
		exit(EXIT_FAILURE);
	}

	options->num_lines = l;
}

static int
set_pointer(options_t *options, const char *optarg)
{
	if (optarg && *optarg) {
		options->pointer = optarg;
		return 1;
	}

	return 0;
}

static int
set_marker(options_t *options, const char *optarg)
{
	if (optarg && *optarg) {
		options->marker = optarg;
		return 1;
	}

	return 0;
}

static void
set_scrolloff(options_t *options, const char *optarg)
{
	if (optarg && *optarg >= '0' && *optarg <= '9')
		options->scrolloff = atoi(optarg);
	else if (optarg && *optarg == 'a' && strcmp(optarg, "auto") == 0)
		options->scrolloff = -1;
}

static void
print_version(void)
{
	puts(VERSION);
	exit(EXIT_SUCCESS);
}

static int
set_separator(options_t *options, const char *optarg)
{
	options->show_info = 1;
	if (optarg && *optarg)
		options->separator = optarg;
	return 1;
}

static void
set_case_sensitivy_mode(options_t *options, const char *optarg)
{
	if (!optarg || !*optarg)
		return;

	if (strcmp(optarg, "respect") == 0 || strcmp(optarg, "sensitive") == 0)
		options->case_sens_mode = CASE_SENSITIVE;
	else if (strcmp(optarg, "ignore") == 0 || strcmp(optarg, "insensitive") == 0)
		options->case_sens_mode = CASE_INSENSITIVE;
	else if (strcmp(optarg, "smart") == 0)
		options->case_sens_mode = CASE_SMART;
}

void
options_parse(options_t *options, int argc, char *argv[])
{
	options_init(options);
	int pointer_set = 0;
	int marker_set = 0;
	int separator_set = 0;

	int c;
	while ((c = getopt_long(argc, argv, "0ce:hij:l:mM:p:P:q:rt:sv",
	longopts, NULL)) != -1) {
		switch (c) {
		case '0': options->input_delimiter = '\0'; break;
		case 'c': options->cycle = 1; break;
		case 'e': options->filter = optarg; break;
		case 'h': usage(); exit(EXIT_SUCCESS);
		case 'i': options->show_info = 1; break;
		case 'j': set_workers(options, optarg); break;
		case 'l': set_lines(options, optarg); break;
		case 'm': options->multi = 1; break;
		case 'M': set_max_items(options, optarg); break;
		case 'p': options->prompt = optarg; break;
		case 'P': set_padding(options, optarg); break;
		case 'q': options->init_search = optarg; break;
		case 'r': options->reverse = 1; break;
		case 't': options->tty_filename = optarg; break;
		case 's': options->show_scores = 1;	break;
		case 'v': print_version(); break;
		case OPT_CASE: set_case_sensitivy_mode(options, optarg); break;
		case OPT_COLOR: options->color = optarg; break;
		case OPT_LEFT_ABORTS: options->left_aborts = 1; break;
		case OPT_MARKER: marker_set = set_marker(options, optarg); break;
		case OPT_NO_CLEAR: options->clear = 0; break;
		case OPT_NO_COLOR: options->no_color = 1; break;
		case OPT_NO_SORT: options->sort = 0; break;
		case OPT_NO_UNICODE: options->unicode = 0; break;
		case OPT_POINTER: pointer_set = set_pointer(options, optarg); break;
		case OPT_PRINT_NULL: options->print_null = 1; break;
		case OPT_RIGHT_ACCEPTS: options->right_accepts = 1; break;
		case OPT_SCROLLOFF: set_scrolloff(options, optarg); break;
		case OPT_SEPARATOR: separator_set = set_separator(options, optarg); break;
		case OPT_TAB_ACCEPTS: options->tab_accepts = 1; break;
		default: usage(); exit(EXIT_SUCCESS);
		}
	}

	if (optind != argc) {
		usage();
		exit(EXIT_FAILURE);
	}

	if (separator_set == 1 && !options->separator)
		options->separator = options->unicode == 0 ? DEFAULT_SEPARATOR
			: DEFAULT_SEPARATOR_UNICODE;

	if (options->unicode != 0) {
		if (pointer_set == 0)
			options->pointer = DEFAULT_POINTER_UNICODE;
		if (marker_set == 0)
			options->marker = DEFAULT_MARKER_UNICODE;
	}
}
