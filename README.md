# Babyc

[![Build Status](https://travis-ci.org/Wilfred/babyc.svg?branch=master)](https://travis-ci.org/Wilfred/babyc)

An educational foray into compiler writing. Written in C, compiling C
to x86 assembly.

Technically targetting C11, but we will implement such a small subset
of C that it's academic.

Current feature set:

* main function (no function definition yet)
* positive integers (no other types yet)
* integer constants
* logical negation (`!FOO`)
* bitwise negation (`~FOO`)
* addition (`foo + bar`)

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

### Debugging

If you're debugging a compiled program that segfaults, you may want to
simply read the out.s file.

To use gdb (given we have no signal table, function prologues or other
conveniences), do the following:

```
$ gdb out
(gdb) run
... it segfaults
(gdb) layout asm
... shows which line the segfault occurred on
(gdb) info registers
... shows the current state of the registers (`layout reg' also
... provides this data)
```
