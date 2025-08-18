## 0.4 (Aug 18, 2025)
## BUG FIXES
* No entry emitted (upon pressing <kbd>Enter</kbd>) after deselecting (via <kbd>Tab</kbd>) all entries.
* Crash with `--read-null`.
* Info not padded when not running with `--reverse`.
* `--show-scores` not honoring `--pad`.
* Missing options in the manpage compared to `--help`.
* <kbd>PgUp</kbd>/<kbd>PgDn</kbd> ignores `--cycle` when selection is first/last respectively.
* If running with --show-scores, the score for a full match is 'inf'.
* Default colors not loaded if **FNF_COLORS** is set.
* 16-colors are not actually 4bit, but 256-color.
* `--color=sel-fg:-1` breaks the interface.

## IMPROVEMENTS
* Add `--no-clear` option to prevent clearing the interface on exit.
* Reduced flickering when scrolling the list of matches.
* Use `--separator` to print a horizontal separator on the info line.
* Add number of selected items to the info line.
* Customize the info line, item score, item foreground, gutter, separator, and query colors. E.g.: `--color="info:222,score:128,fg:245:1,gutter:236,query:74,separator:240"`.
* Scores are surrounded by square brackets instead of parentheses.
* Improved default colors
* Scrolloff set by default to half available entries (run with `--scroll-off=0` to restore the previous behavior). 
* Use <kbd>Ctrl+f</kbd> and <kbd>Ctrl+b</kbd> to move the cursor forward and backward respectively.
* Use <kbd>Ctrl+PgUp</kbd> and <kbd>Ctrl+PgDn</kbd> jump to the first and last available items respectively.
* Highlight matches forward, instead of backwards.
* Highly improved Unicode matching.
* Add `-r` and `-c` short options for `--reverse` and `--cycle` respectively.
* `--lines` defaults to half of terminal height (instead of 10).
* Add `--height` as an alias for `--lines`.
* Set case sensitivity mode with `--case=respect|ignore|smart` (defaults to `smart`).
* Add `--no-bold` to disable bold colors.
* Set the base color scheme using `--color-scheme=dark|light|16` (defaults to `dark`).
* Use `--ghost` to set ghost text when input is empty.

## MISC
* Rename `--pad` option to `--padding` (`--pad` still works).

---

## 0.3.1 (Jul 24, 2025)

### BUG FIXES
* <kbd>Home</kbd> and <kbd>End</kbd> keys not working in VTE and rxvt terminals.
* The <kbd>Del</kbd> key does not work.
* Wide Unicode chacracters (like CJK) break the interface.

### IMPROVEMENTS
* <kbd>Ctrl+d</kbd> works the same as <kbd>Del</kbd>, but aborts fnf if the qeury is empty.

---

## 0.3 (Jul 21, 2025)

### BUG FIXES
* Selection foreground color ignored when entry is already colorized.
* Cursor flickering on VTE terminals when running with `--reverse`.
* --show-info not working with `--reverse`.
* Phantom lines when running with `--reverse` and `--lines` is set to cover the entire terminal screen.
* `--lines=auto` not working with `--show-info` (phantom lines).
* <kbd>PgUp</kbd> and <kbd>PgDn</kbd> do cycle even if cycling is disabled.

### IMPROVEMENTS
* Set custom colors (256/truecolor) using `--color` or **$FNF_COLORS** (see the manpage).
* Default selection color changed to bold white on gray for enhanced contrast.
* Allow the use of strings, including Unicode, for pointer and marker (e.g. `fnf --pointer="┃" --pointer="✔"`).
* Default to Unicode decorations (disable with `--no-unicode`).
* Add `--print-null` (print output delimited by NUL characters).
* Minimize screen flickering.
* Add `--no-sort` option to get unsorted results.
* Use <kbd>Shift-TAB</kbd> (btab) to (un)mark the current item and select the previous item.

---

## 0.2 (Jul 9, 2025)

### BUG FIXES
* Matching characters colorized even if running with --no-color
* Left and Right key not working in the prompt when running with --reverse
* Prompt color ignored when running with --reverse
* Several memory warnings

---

## 0.1 (Jun 6, 2023)

Initial release
