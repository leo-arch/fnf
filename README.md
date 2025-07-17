# FNF
A fast, simple fuzzy finder for the terminal.

Born as a fork of [fzy](https://github.com/jhawthorn/fzy) (originaly intended to make _fzy_ work with [the clifm file manager](https://github.com/leo-arch/clifm)), **fnf** (recursive acronym for **fnf's not fzy**) adds a few new features to the original _fzy_, including color support (256 and 24-bit colors), padding, and multi-selection. Consult the manpage for more information.

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

The `PREFIX` environment variable can be used to specify the install location (the default is `/usr/local`).

### Use with [clifm](https://github.com/leo-arch/clifm)

Just run **clifm** as follows:

```sh
clifm --fnftab
```

**Note**: You need at least **clifm** 1.12.9.

## Sorting

**Fnf** attempts to present the best matches first. The following considerations are weighted when sorting:

It prefers consecutive characters: `file` will match <tt><b>file</b></tt> over <tt><b>fil</b>t<b>e</b>r</tt>.

It prefers matching the beginning of words: `amp` is likely to match <tt><b>a</b>pp/<b>m</b>odels/<b>p</b>osts.rb</tt>.

It prefers shorter matches: `abce` matches <tt><b>abc</b>d<b>e</b>f</tt> over <tt><b>abc</b> d<b>e</b></tt>.

It prefers shorter candidates: `test` matches <tt><b>test</b>s</tt> over <tt><b>test</b>ing</b></tt>.
