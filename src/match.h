/*
 * This file is part of fnf
 *
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2014-2022 John Hawthorn <john.hawthorn@gmail.com>
 * SPDX-FileCopyrightText: 2022-2026, L. Abramovich <leo.clifm@outlook.com>
*/

/* match.h */

#ifndef MATCH_H
#define MATCH_H

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef double score_t;
#define SCORE_MAX ((score_t)INFINITY)
#define SCORE_MIN ((score_t)(-INFINITY))

#define MATCH_MAX_LEN 1024

int has_match(const char *needle, const char *haystack);
score_t match_positions(const char *needle, const char *haystack,
	size_t *positions);
score_t match(const char *needle, const char *haystack);

#ifdef __cplusplus
}
#endif

#endif /* MATCH_H */
