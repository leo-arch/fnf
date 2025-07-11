# FNF
A fast, simple fuzzy finder with an advanced scoring algorithm.

Born as a fork of [fzy](https://github.com/jhawthorn/fzy) (originaly intended to make _fzy_ work with [the clifm file manager](https://github.com/leo-arch/clifm)), **fnf** (recursive acronym for **fnf's not fzy**) adds a few new features to the original _fzy_, including basic color support, padding, and multi-selection. Consult the manpage for more information.

<!---
![](http://i.hawth.ca/u/fzy_animated_demo.svg)

## Why use this over fzf, pick, selecta, ctrlp, ...?

fzy is faster and shows better results than other fuzzy finders.

Most other fuzzy matchers sort based on the length of a match. fzy tries to
find the result the user intended. It does this by favouring matches on
consecutive letters and starts of words. This allows matching using acronyms or
different parts of the path.

A gory comparison of the sorting used by fuzzy finders can be found in [ALGORITHM.md](ALGORITHM.md)

fzy is designed to be used both as an editor plugin and on the command line.
Rather than clearing the screen, fzy displays its interface directly below the current cursor position, scrolling the screen if necessary.
--->

## Installation

[![Packaging status](https://repology.org/badge/vertical-allrepos/fnf.svg)](https://repology.org/project/fnf/versions)

**Fnf** is availible in multiple package repositories. If not packaged for your system, perform a manual installation as follows:

```sh
mkdir build && cd build
git clone https://github.com/leo-arch/fnf
cd fnf
make
sudo make install
```

The `PREFIX` environment variable can be used to specify the install location,
the default is `/usr/local`.

<!---
## Usage

fzy is a drop in replacement for [selecta](https://github.com/garybernhardt/selecta), and can be used with its [usage examples](https://github.com/garybernhardt/selecta#usage-examples).

### Use with Vim

fzy can be easily integrated with vim.

``` vim
function! FzyCommand(choice_command, vim_command)
  try
    let output = system(a:choice_command . " | fzy ")
  catch /Vim:Interrupt/
    " Swallow errors from ^C, allow redraw! below
  endtry
  redraw!
  if v:shell_error == 0 && !empty(output)
    exec a:vim_command . ' ' . output
  endif
endfunction

nnoremap <leader>e :call FzyCommand("find . -type f", ":e")<cr>
nnoremap <leader>v :call FzyCommand("find . -type f", ":vs")<cr>
nnoremap <leader>s :call FzyCommand("find . -type f", ":sp")<cr>
```

Any program can be used to filter files presented through fzy. [ag (the silver searcher)](https://github.com/ggreer/the_silver_searcher) can be used to ignore files specified by `.gitignore`.

``` vim
nnoremap <leader>e :call FzyCommand("ag . --silent -l -g ''", ":e")<cr>
nnoremap <leader>v :call FzyCommand("ag . --silent -l -g ''", ":vs")<cr>
nnoremap <leader>s :call FzyCommand("ag . --silent -l -g ''", ":sp")<cr>
```
--->

### Use with [clifm](https://github.com/leo-arch/clifm)

Just run **clifm** as follows:

```sh
clifm --fnftab
```

**Note**: You need at least **clifm** 1.12.9. If running a previous version use [our old fzy fork](https://github.com/leo-arch/fzy) (the option is named `--fzytab` instead of `--fnftab`).

## Sorting

**Fnf** attempts to present the best matches first. The following considerations are weighted when sorting:

It prefers consecutive characters: `file` will match <tt><b>file</b></tt> over <tt><b>fil</b>t<b>e</b>r</tt>.

It prefers matching the beginning of words: `amp` is likely to match <tt><b>a</b>pp/<b>m</b>odels/<b>p</b>osts.rb</tt>.

It prefers shorter matches: `abce` matches <tt><b>abc</b>d<b>e</b>f</tt> over <tt><b>abc</b> d<b>e</b></tt>.

It prefers shorter candidates: `test` matches <tt><b>test</b>s</tt> over <tt><b>test</b>ing</b></tt>.
