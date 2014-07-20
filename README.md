# Babyc

[![Build Status](https://travis-ci.org/Wilfred/babyc.svg?branch=master)](https://travis-ci.org/Wilfred/babyc)

An educational foray into compiler writing. Written in C, compiling C
to x86 assembly.

Technically targetting C11, but we will implement such a small subset
of C that it's academic.

Current feature set:

* main function (no function definition yet)
* integers (no other types yet)
* integer constants
* logical negation (`!FOO`)
* bitwise negation (`~FOO`)

## License

GPL v2 license.

## Usage

```
# Compile the compiler.
$ make
# Run it, producing an assembly file.
$ babyc test_programs/just_a_number__return_1.c
# Use the GNU toolchain to assemble and link.
$ ./link
```
