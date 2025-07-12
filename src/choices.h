/* choices.h */

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

#ifndef CHOICES_H
#define CHOICES_H CHOICES_H

#include <stdio.h>

#include "match.h"
#include "options.h"

struct scored_result {
	score_t score;
	const char *str;
};

typedef struct {
	char *buffer;
	size_t buffer_size;

	size_t capacity;
	size_t size;

	const char **strings;
	struct scored_result *results;

	size_t available;
	size_t selection;

	unsigned int worker_count;
} choices_t;

void choices_init(choices_t *c, const options_t *options);
void choices_fread(choices_t *c, FILE *file, const char input_delimiter);
void choices_destroy(choices_t *c);
void choices_add(choices_t *c, const char *choice);
size_t choices_available(const choices_t *c);
void choices_search(choices_t *c, const char *search);
const char *choices_get(const choices_t *c, const size_t n);
score_t choices_getscore(const choices_t *c, const size_t n);
void choices_prev(choices_t *c);
void choices_next(choices_t *c);

#endif
