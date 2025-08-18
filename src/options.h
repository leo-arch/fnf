/* options.h */

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
