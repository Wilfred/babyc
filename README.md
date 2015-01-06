# Babyc

[![Build Status](https://travis-ci.org/Wilfred/babyc.svg?branch=master)](https://travis-ci.org/Wilfred/babyc)

An educational foray into compiler writing. Written in C, compiling C
to x86 assembly
([handy x86 reference site](http://x86.renejeschke.de/),
[assembly directives reference](https://www.sourceware.org/binutils/docs-2.12/as.info/Pseudo-Ops.html),
[System V ABI reference](http://www.uclibc.org/docs/psABI-i386.pdf)).

Technically targetting C11
([standard PDF](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)),
but we will implement such a small subset of C that it's academic.

Current feature set:

* positive integers (no other types yet)
* integer constants
* logical negation (`!FOO`)
* bitwise negation (`~FOO`)
* addition (`foo + bar`)
* subtraction (`foo - bar` binary only)
* multiplication (`foo * bar`)
* less than comparison (`foo < bar`)
* comments (`// foo` and `/* foo */`)
* sequences of statements (`foo; bar`)
* return statements
* if statements (`if (foo) { bar }`, no `else` yet)
* local variables (`int` only, function scope only, must be
  initialised)
* variable assignment (`int` only)
* while loops (`while (foo) { bar }`)
* function calls (only `int foo()` i.e. no arguments, returning int)
* preprocessor usage (we shell out to gcc)

## License

GPL v2 license.

## Usage

Compiling babyc:

```
# Compile the compiler.
$ make
```

Usage:

```
# Run it, producing an assembly file.
$ build/babyc test_programs/immediate__return_1.c
# Use the GNU toolchain to assemble and link.
$ ./link
```

Viewing the code after preprocessing:

```
$ build/babyc --dump-expansion test_programs/if_false__return_2.c
```


Viewing the AST:

```
$ build/babyc --dump-ast test_programs/if_false__return_2.c
```

Running tests:

```
$ make test
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

If you want to debug a program that doesn't segfault, you can set a
breakpoint to the entrypoint:

```
$ gdb out
(gdb) info files
    ...
    Entry point: 0x80000000
    ...
(gdb) break *0x80000000
(gdb) run
```

## Improving code quality

The make command will generate warnings, fix them. You can also run
with clang-analyzer to catch further issues:

```
$ scan-build make
```
