/* match.c */

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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h> /* uint8_t */

#include "match.h"
#include "bonus.h"
#include "colors.h"
#include "tty_interface.h" /* g_case_sensitive */

#define TOLOWER(c) (((c) >= 'A' && (c) <= 'Z') ? (c) | 32 : tolower(c))
#define TOUPPER(c) (((c) >= 'a' && (c) <= 'z') ? (c) & ~32 : toupper(c))

static char *
strcasechr(const char *s, int c)
{
	char lower_c = TOLOWER(c);
	char upper_c = TOUPPER(c);
	const char accept[3] = {lower_c, upper_c, '\0'};
	return strpbrk(s, accept);
}

static const char *
skip_sgr_sequences(const char *str)
{
	const char *s = str;

	while (IS_SGR_START(s)) {
		s += 2;
		while (*s && *s != 'm' && IS_SGR_CHAR(*s))
			s++;
		if (*s == 'm')
			s++;
	}

	return s;
}

int
has_match(const char *needle, const char *haystack)
{
	/* Skip initial SGR sequence from haystack. */
	if (*haystack == KEY_ESC) {
		haystack = skip_sgr_sequences(haystack);
		if (!*haystack)
			return 0;
	}

	/* Set a pointer to the beginning of the last SGR sequence. */
	const char *escape_key = haystack;
	while (*escape_key) {
		if (IS_SGR_START(escape_key))
			break;
		escape_key++;
	}

	char *(*strchr_func)(const char *, int);
	strchr_func = g_case_sensitive == 0 ? strcasechr : strchr;

	/* Inspect haystack up to the beginning of the last SGR sequence. */
	while (*needle) {
		const char nch = *needle++;
		if (!(haystack = strchr_func(haystack, nch)) || haystack >= escape_key)
			return 0;

		haystack++;
	}

	return 1;
}

#define SWAP(x, y, T) do { T SWAP = x; x = y; y = SWAP; } while (0)
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

struct match_t {
	score_t match_bonus[MATCH_MAX_LEN];
	char lower_needle[MATCH_MAX_LEN];
	char lower_haystack[MATCH_MAX_LEN];
	size_t needle_len;
	size_t haystack_len;
};

static void
precompute_bonus(const char *haystack, score_t *match_bonus)
{
	/* Which positions are beginning of words */
	char last_char = '/';
	for (size_t i = 0; haystack[i]; i++) {
		const char c = haystack[i];
		match_bonus[i] = COMPUTE_BONUS(last_char, c);
		last_char = c;
	}
}

static int
tolower_dummy(int c)
{
	return c;
}

static void
setup_match_struct(struct match_t *match, const char *needle,
	const char *haystack)
{
	/* Skip leading and trailing SGR color sequences from HAYSTACK. */
	if (*haystack == KEY_ESC) {
		haystack = skip_sgr_sequences(haystack);
		char *esc = strchr(haystack, KEY_ESC);
		match->haystack_len = (esc && esc[1] == '[') ? (size_t)(esc - haystack)
			: strlen(haystack);
	} else {
		match->haystack_len = strlen(haystack);
	}

	match->needle_len = strlen(needle);

	if (match->haystack_len > MATCH_MAX_LEN
	|| match->needle_len > match->haystack_len)
		return;

	int (*tolower_func)(int);
	tolower_func = g_case_sensitive == 0 ? tolower : tolower_dummy;

	for (size_t i = 0; i < match->needle_len; i++)
		match->lower_needle[i] = tolower_func(needle[i]);

	for (size_t i = 0; i < match->haystack_len; i++)
		match->lower_haystack[i] = tolower_func(haystack[i]);

	precompute_bonus(haystack, match->match_bonus);
}

static inline void
match_row(const struct match_t *match, const size_t row, score_t *curr_D,
	score_t *curr_M, const score_t *last_D, const score_t *last_M)
{
	const size_t n = match->needle_len;
	const size_t m = match->haystack_len;
	const size_t i = row;

	const char *lower_needle = match->lower_needle;
	const char *lower_haystack = match->lower_haystack;
	const score_t *match_bonus = match->match_bonus;

	score_t prev_score = SCORE_MIN;
	const score_t gap_score = i == n - 1 ? SCORE_GAP_TRAILING : SCORE_GAP_INNER;

	for (size_t j = 0; j < m; j++) {
		if (lower_needle[i] == lower_haystack[j]) {
			score_t score = SCORE_MIN;
			if (!i) {
				score = (j * SCORE_GAP_LEADING) + match_bonus[j];
			} else if (j) { /* i > 0 && j > 0 */
				score = MAX(
					last_M[j - 1] + match_bonus[j],
					/* consecutive match, doesn't stack with match_bonus */
					last_D[j - 1] + SCORE_MATCH_CONSECUTIVE);
			}
			curr_D[j] = score;
			curr_M[j] = prev_score = MAX(score, prev_score + gap_score);
		} else {
			curr_D[j] = SCORE_MIN;
			curr_M[j] = prev_score = prev_score + gap_score;
		}
	}
}

score_t
match(const char *needle, const char *haystack)
{
	if (!*needle)
		return SCORE_MIN;

	struct match_t match;
	setup_match_struct(&match, needle, haystack);

	const size_t n = match.needle_len;
	const size_t m = match.haystack_len;

	if (m > MATCH_MAX_LEN || n > m) {
		/* Unreasonably large candidate: return no score
		 * If it is a valid match it will still be returned, it will
		 * just be ranked below any reasonably sized candidates. */
		return SCORE_MIN;
	} else if (n == m) {
		/* Since this method can only be called with a haystack which
		 * matches needle. If the lengths of the strings are equal the
		 * strings themselves must also be equal (ignoring case). */
		return SCORE_MAX;
	}

	/* D[][] Stores the best score for this position ending with a match.
	 * M[][] Stores the best possible score at this position. */
	score_t D[2][MATCH_MAX_LEN], M[2][MATCH_MAX_LEN];

	score_t *last_D = D[0];
	score_t *last_M = M[0];
	score_t *curr_D = D[1];
	score_t *curr_M = M[1];

	for (size_t i = 0; i < n; i++) {
		match_row(&match, i, curr_D, curr_M, last_D, last_M);

		SWAP(curr_D, last_D, score_t *);
		SWAP(curr_M, last_M, score_t *);
	}

	return last_M[m - 1];
}

static uint8_t utf8_len_table[256] = {0};

static void
init_utf8_len_table(void)
{
	for (size_t i = 0; i < 256; i++) {
		if (i < 128)
			utf8_len_table[i] = 1;
		else if (i >= 192 && i <= 223)
			utf8_len_table[i] = 2;
		else if (i >= 224 && i <= 239)
			utf8_len_table[i] = 3;
		else if (i >= 240 && i <= 247)
			utf8_len_table[i] = 4;
		else
			utf8_len_table[i] = 0;
	}
}

/* Return 1 if the first wide character in HAYSTACK matches the first wide
 * character in NEEDLE, or 0 otherwise. */
static inline int
compare_utf8_chars(const char *haystack, const char *needle)
{
	const size_t needle_len = utf8_len_table[(uint8_t)*needle];
	if (needle_len == 0)
		return 0;

	for (size_t i = 0; i < needle_len; i++) {
		if (haystack[i] != needle[i])
			return 0;
	}

	return 1;
}

static score_t
fill_full_match_positions(size_t *positions, const char *needle, const size_t n)
{
	const uint8_t *u_needle = (const uint8_t *)needle;

	for (size_t i = 0, p = 0; i < n; i++) {
		positions[p++] = i;

		/* If multi-byte, skip the remaining bytes. */
		const size_t char_len = utf8_len_table[u_needle[i]];
		if (char_len >= 2) /* Multi-byte character */
			i += char_len - 1;	/* -1 because the for-loop will increment i */
	}

	return SCORE_MAX;
}

/* Return the score indicating the degree of match between HAYSTAK and NEEDLE.
 * A higher score indicates a better match.
 * The POSITIONS array is populated with the positions (indices) of the
 * matching characters in HAYSTACK. In case of UTF-8 strings, only the
 * position of the first byte of each matching multi-byte character is added
 * to the POSITIONS array.
 * All parameters are guaranteed to be non-null. */
score_t
match_positions(const char *needle, const char *haystack, size_t *positions)
{
	/* When initialized, utf8_len_table[0] is 1 */
	if (utf8_len_table[0] == 0)
		init_utf8_len_table();

	if (!*needle)
		return SCORE_MIN;

	struct match_t match;
	setup_match_struct(&match, needle, haystack);

	const size_t n = match.needle_len;
	const size_t m = match.haystack_len;

	if (n == 0 || m == 0) {
		return SCORE_MIN;
	} else if (m > MATCH_MAX_LEN || n > m) {
		/* Unreasonably large candidate: return no score.
		 * If it is a valid match, it will still be returned, it will
		 * just be ranked below any reasonably sized candidates. */
		return SCORE_MIN;
	} else if (n == m) {
		/* Since this method can only be called with a haystack which
		 * matches needle, if the lengths of the strings are equal, then
		 * the strings themselves must also be equal (ignoring case). */
		return fill_full_match_positions(positions, needle, n);
	}

	/* D[][] Stores the best score for this position ending with a match.
	 * M[][] Stores the best possible score at this position. */
	score_t (*D)[MATCH_MAX_LEN] = calloc(n, sizeof(*D));
	score_t (*M)[MATCH_MAX_LEN] = calloc(n, sizeof(*M));
	if (!M || !D) {
		free(D); free(M);
		return SCORE_MIN;
	}

	score_t *last_D = NULL, *last_M = NULL;
	score_t *curr_D = NULL, *curr_M = NULL;

	for (size_t i = 0; i < n; i++) {
		curr_D = &D[i][0];
		curr_M = &M[i][0];

		match_row(&match, i, curr_D, curr_M, last_D, last_M);

		last_D = curr_D;
		last_M = curr_M;
	}

	/* Backtrace to find the positions of optimal matching. */
	size_t p = 0; /* Current positions index. */
	int match_required = 0;

	for (size_t i = 0, j = 0; i < n; i++) {
		for (; j < m; j++) {
			/* There may be multiple paths which result in the optimal weight.
			 * For simplicity, we will pick the first one we encounter, the
			 * first in the candidate string. */
			if (D[i][j] == SCORE_MIN ||
			(!match_required && D[i][j] != M[i][j]))
				continue;

			/* If this score was determined using SCORE_MATCH_CONSECUTIVE, the
			 * next character MUST be a match. */
			match_required = i && j &&
				(M[i][j] == (D[i - 1][j - 1] + SCORE_MATCH_CONSECUTIVE));

			/* Check if the current char in haystack matches the current
			 * char in needle. */
			const size_t char_len = utf8_len_table[(uint8_t)haystack[j]];
			if (char_len < 2) { /* ASCII character (or invalid) */
				positions[p++] = j++;
				break;
			}

			if (compare_utf8_chars(&haystack[j], &needle[i])) {
				/* Mark the position of the first byte of the
				 * matching multi-byte character. */
				positions[p++] = j;

				/* Skip remaining bytes of the multi-byte character,
				 * both in haystack (j) and in needle (i). */
				j += char_len;
				/* -1 because the main for-loop will increment i */
				i += char_len - 1;
				break;
			}
		}
	}

	score_t result = M[n - 1][m - 1];
	free(M);
	free(D);
	return result;
}
