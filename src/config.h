/*
 * This file is part of fnf
 *
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2014-2022 John Hawthorn <john.hawthorn@gmail.com>
 * SPDX-FileCopyrightText: 2022-2026, L. Abramovich <leo.clifm@outlook.com>
*/

/* config.h */

#ifndef CONFIG_H
#define CONFIG_H

#define SCORE_GAP_INNER -0.01
#define SCORE_GAP_LEADING -0.005
#define SCORE_GAP_TRAILING -0.005
#define SCORE_MATCH_CAPITAL 0.7
#define SCORE_MATCH_CONSECUTIVE 1.0
#define SCORE_MATCH_DOT 0.6
#define SCORE_MATCH_SLASH 0.9
#define SCORE_MATCH_WORD 0.8

#define CASE_INSENSITIVE 0
#define CASE_SENSITIVE   1
#define CASE_SMART       2

#define DEFAULT_AUTO_LINES 0
#define DEFAULT_CASE_SENSITIVITY_MODE CASE_SMART
#define DEFAULT_CLEAR 1
#define DEFAULT_COLORS_DARK "fg:-1,ghost:243,gutter:-1,hl:216,info:144," \
	"marker:72,pointer:124,prompt:74:1,query:-1,score:102,sel-bg:236,"   \
	"sel-fg:7:1,separator:240" /* This is the default color scheme */
#define DEFAULT_COLORS_LIGHT "fg:-1,ghost:243,gutter:-1,hl:90,info:233," \
	"marker:160,pointer:6:2,prompt:21:1,query:-1,score:238,sel-bg:251,"   \
	"sel-fg:0,separator:238"
#define DEFAULT_COLORS_16 "fg:-1,ghost:7:2,gutter:-1,hl:6,info:7:2,marker:2," \
	"pointer:1:1,prompt:6:1,query:-1,score:7:2,sel-bg:0,sel-fg:7:1,separator:7:2"
#define DEFAULT_CYCLE 0
#define DEFAULT_DELIMITER '\n'
#define DEFAULT_FILTER NULL
#define DEFAULT_INIT_SEARCH NULL
#define DEFAULT_LEFT_ABORTS 0
#define DEFAULT_MARKER "*"
#define DEFAULT_MARKER_UNICODE "✔"
#define DEFAULT_MAX_ITEMS -1 /* Unlimited */
#define DEFAULT_MULTI 0
#define DEFAULT_NO_BOLD 0
#define DEFAULT_NO_COLOR 0
#define DEFAULT_NUM_LINES 10
#define DEFAULT_PAD 0
#define DEFAULT_POINTER ">"
#define DEFAULT_POINTER_UNICODE "▌"
#define DEFAULT_PRINT_NULL 0
#define DEFAULT_PROMPT "> "
#define DEFAULT_REVERSE 0
#define DEFAULT_RIGHT_ACCEPTS 0
#define DEFAULT_SCORES 0
#define DEFAULT_SCROLLOFF -1 /* auto */
#define DEFAULT_SEPARATOR "-"
#define DEFAULT_SEPARATOR_UNICODE "─"
#define DEFAULT_SORT 1
#define DEFAULT_SHOW_INFO 0
#define DEFAULT_TAB_ACCEPTS 0
#define DEFAULT_TTY "/dev/tty"
#define DEFAULT_UNICODE 1
#define DEFAULT_WORKERS 0 /* 0: Number of CPUs */

/* If running without colors (--no-color or NO_COLOR) */
#define SELECTION_NOCOLOR "\x1b[7m" /* Invert */
#define HIGHLIGHT_NOCOLOR "\x1b[4m" /* Underline */

#endif /* CONFIG_H */
