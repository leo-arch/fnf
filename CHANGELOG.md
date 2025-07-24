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
