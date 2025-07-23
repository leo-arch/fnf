
<p align="center"><img src="https://i.postimg.cc/8Csfj3cq/fnf.png" width="500" height="400"></p>
<h2 align="center">A fast, simple fuzzy finder for the terminal</h2>

<p align="center">
<a href="https://github.com/leo-arch/fnf/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-MIT-red?style=flat"/></a>
<a href="https://github.com/leo-arch/fnf/releases"><img alt="GitHub release (latest by date)" src="https://img.shields.io/github/v/release/leo-arch/fnf"></a>
<a><img src="https://img.shields.io/github/commits-since/leo-arch/fnf/latest"></a>
</p>

<p align="center">
<a href="https://github.com/leo-arch/fnf/actions/workflows/codeql-analysis.yml"><img src="https://github.com/leo-arch/fnf/actions/workflows/codeql-analysis.yml/badge.svg?branch=master"></a>
<a href="https://app.codacy.com/gh/leo-arch/fnf/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade"><img src="https://app.codacy.com/project/badge/Grade/4a52165e839f499587e7cf798da2453d"/></a>
</p>

---
## Description

**Fnf** (short for **fnf's not fzy**) is a fork of the original [fzy tool](https://github.com/jhawthorn/fzy), which was initially designed to work with the [clifm file manager](https://github.com/leo-arch/clifm).
 This version introduces several new features, including support for 256  and 24-bit colors, customizable padding, and multi-selection capabilities. For detailed usage instructions and additional information, please refer to the manpage.

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

> [!NOTE]
> You need at least **clifm** 1.12.9.

## Sorting

**Fnf** attempts to present the best matches first. The following considerations are weighted when sorting:

It prefers consecutive characters: `file` will match <tt><b>file</b></tt> over <tt><b>fil</b>t<b>e</b>r</tt>.

It prefers matching the beginning of words: `amp` is likely to match <tt><b>a</b>pp/<b>m</b>odels/<b>p</b>osts.rb</tt>.

It prefers shorter matches: `abce` matches <tt><b>abc</b>d<b>e</b>f</tt> over <tt><b>abc</b> d<b>e</b></tt>.

It prefers shorter candidates: `test` matches <tt><b>test</b>s</tt> over <tt><b>test</b>ing</b></tt>.
