# FNF
A fast, simple fuzzy finder for the terminal.

**Fnf** (short for **fnf's not fzy**) is an enhanced fork of the original [fzy tool](https://github.com/jhawthorn/fzy), which was initially designed to work with the [clifm file manager](https://github.com/leo-arch/clifm). This version introduces several new features, including support for 256 and 24-bit colors, customizable padding, and multi-selection capabilities. For detailed usage instructions and additional information, please refer to the manpage.

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

> [!NOTE]
> If not running on Linux, you may need to use `gmake` instead of `make`.

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
