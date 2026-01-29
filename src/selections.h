/*
 * This file is part of fnf
 *
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2022-2026, L. Abramovich <leo.clifm@outlook.com>
*/

/* selections.h */

#ifndef SELECTIONS_H
#define SELECTIONS_H

#include "tty_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

void deselect_entry(const char *name, tty_interface_t *state);
void free_selections(tty_interface_t *state);
int  is_selected(const char *name);
void print_selections(tty_interface_t *state);
void save_selection(const char *name, sel_t *selection);

#ifdef __cplusplus
}
#endif

#endif /* SELECTIONS_H */
