/*
 * This file is part of fnf
 *
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2014-2022 John Hawthorn <john.hawthorn@gmail.com>
 * SPDX-FileCopyrightText: 2022-2026, L. Abramovich <leo.clifm@outlook.com>
*/

/* keybindings.h */

#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include "tty_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

void handle_input(tty_interface_t *state, const char *s,
	const int handle_ambiguous_key);

#ifdef __cplusplus
}
#endif

#endif /* KEYBINDINGS_H */
