/* config.h */

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
#define DEFAULT_COLORS_DARK "fg:-1,gutter:-1,hl:216,info:144,marker:72," \
	"pointer:124,prompt:74:1,query:-1,score:102,sel-bg:236,"             \
	"sel-fg:7:1,separator:240" /* This is the default color scheme */
#define DEFAULT_COLORS_16 "fg:-1,gutter-1,hl:6,info:2,marker:2,prompt:6:1," \
	"pointer:1,query:7,score:8,sel-bg:0,sel-fg:7:1,separator:8"
#define DEFAULT_COLORS_LIGHT "fg:0,gutter:-1,hl:5,info:2:2,marker:1," \
	"prompt:4:1,pointer:6:2,query:0,score:8,sel-bg:7,sel-fg:0,separator:8"
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
