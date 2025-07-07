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
#define OPTIONS_H OPTIONS_H

typedef struct {
	int benchmark;
	const char *filter;
	const char *init_search;
	const char *tty_filename;
	int show_scores;
	unsigned int num_lines;
	int auto_lines;
	unsigned int scrolloff;
	const char *prompt;
	unsigned int workers;
	char input_delimiter;
	int show_info;
	int pad;
	int multi;
	char pointer;
	char marker;
	int cycle;
	int tab_accepts;
	int right_accepts;
	int left_aborts;
	int no_color;
	int reverse;
} options_t;

void options_init(options_t *options);
void options_parse(options_t *options, int argc, char *argv[]);

#endif
