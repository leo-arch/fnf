/*
 * This file is part of fnf
 *
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2022-2026, L. Abramovich <leo.clifm@outlook.com>
*/

/* colors.h */

#ifndef COLORS_H
#define COLORS_H

/* Color indices for the colors array. */
#define PROMPT_COLOR    0
#define POINTER_COLOR   1
#define MARKER_COLOR    2
#define SEL_FG_COLOR    3
#define SEL_BG_COLOR    4
#define HIGHLIGHT_COLOR 5
#define INFO_COLOR      6
#define SCORE_COLOR     7
#define FG_COLOR        8
#define QUERY_COLOR     9
#define GUTTER_COLOR    10
#define SEPARATOR_COLOR 11
#define GHOST_COLOR     12
#define COLOR_ITEMS_NUM 13

#define MAX_COLOR_LEN 64

#define TERM_FG_COLOR_RESET 9
#define RESET_ATTR "\x1b[0m" /* Reset attributes */
#define CLEAR_LINE "\x1b[K"

#define COLOR_FIELD_SEP ':'
#define KEY_ESC 27
#define IS_DIGIT(c)     ((c) >= '0' && (c) <= '9')
#define IS_SGR_CHAR(c)  (IS_DIGIT((c)) || (c) == ';' || (c) == '[')
#define IS_SGR_START(s) (*(s) == KEY_ESC && (s)[1] == '[')
#define IS_BG_COLOR(n)  ((n) == SEL_BG_COLOR || (n) == GUTTER_COLOR)

#define IS_UTF8_LEAD_BYTE(c) (((c) & 0xc0) == 0xc0)
#define IS_UTF8_CONT_BYTE(c) (((c) & 0xc0) == 0x80)
#define IS_UTF8_CHAR(c)      (IS_UTF8_LEAD_BYTE((c)) || IS_UTF8_CONT_BYTE((c)))

#include "tty_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char colors[COLOR_ITEMS_NUM][MAX_COLOR_LEN];

char *decolor_name(const char *name, char *color);
void colorize_match(const tty_interface_t *state, const size_t *positions,
	const char *name, const char *orig_color, const pointer_t *pointer,
	const int selected);
void colorize_no_match(tty_t *tty, const char *sel_color, const char *name,
	const pointer_t *pointer);
void set_colors(tty_interface_t *state);

#ifdef __cplusplus
}
#endif

#endif /* COLORS_H */
