/*
 * This file is part of fnf
 *
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2014-2022 John Hawthorn <john.hawthorn@gmail.com>
 * SPDX-FileCopyrightText: 2022-2026, L. Abramovich <leo.clifm@outlook.com>
*/

/* choices.h */

#ifndef CHOICES_H
#define CHOICES_H

#include <stdio.h>

#include "match.h" /* score_t */
#include "options.h"

#ifdef __cplusplus
extern "C" {
#endif

struct scored_result {
	score_t score;
	const char *str;
};

typedef struct {
	char *buffer;
	const char **strings;
	struct scored_result *results;
	size_t buffer_size;
	size_t capacity;
	size_t size;
	size_t available;
	size_t selection;
	size_t worker_count;
} choices_t;

void choices_add(choices_t *c, char *choice);
void choices_init(choices_t *c, const options_t *options);
void choices_fread(choices_t *c, FILE *file, const char input_delimiter,
	const int max_choices);
void choices_destroy(choices_t *c);
size_t choices_available(const choices_t *c);
void choices_search(choices_t *c, const char *search, const int sort);
const char *choices_get(const choices_t *c, const size_t n);
score_t choices_getscore(const choices_t *c, const size_t n);
void choices_prev(choices_t *c);
void choices_next(choices_t *c);

#ifdef __cplusplus
}
#endif

#endif /* CHOICES_H */
