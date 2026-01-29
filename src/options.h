/*
 * This file is part of fnf
 *
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2014-2022 John Hawthorn <john.hawthorn@gmail.com>
 * SPDX-FileCopyrightText: 2022-2026, L. Abramovich <leo.clifm@outlook.com>
*/

/* options.h */

#ifndef OPTIONS_H
#define OPTIONS_H

#define VERSION "0.4"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char *color;
	const char *color_scheme;
	const char *filter;
	const char *ghost;
	const char *init_search;
	const char *tty_filename;
	const char *prompt;
	const char *pointer;
	const char *marker;
	const char *separator;
	size_t num_lines;
	size_t workers;
	int auto_lines;
	int case_sens_mode;
	int clear;
	int cycle;
	int left_aborts;
	int max_items;
	int multi;
	int no_bold;
	int no_color;
	int pad;
	int print_null;
	int reverse;
	int right_accepts;
	int scrolloff;
	int show_scores;
	int show_info;
	int sort;
	int tab_accepts;
	int unicode;
	char input_delimiter;
} options_t;

void options_init(options_t *options);
void options_parse(options_t *options, int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* OPTIONS_H */
