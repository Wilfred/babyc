# Babyc

[![Build Status](https://travis-ci.org/Wilfred/babyc.svg?branch=master)](https://travis-ci.org/Wilfred/babyc)

An educational foray into compiler writing. Written in C, compiling C
to x86 assembly
([handy x86 reference site](http://x86.renejeschke.de/),
[assembly directives reference](https://www.sourceware.org/binutils/docs-2.12/as.info/Pseudo-Ops.html),
[X86 calling conventions](https://en.wikipedia.org/wiki/X86_calling_conventions#cdecl),
[System V ABI reference](http://www.uclibc.org/docs/psABI-i386.pdf)).


Technically targetting C11
([standard PDF](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)),
but we will implement such a small subset of C that it's academic.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc/generate-toc again -->
**Table of Contents**

- [Babyc](#babyc)
    - [Current feature set](#current-feature-set)
    - [License](#license)
    - [Usage](#usage)
        - [Debugging](#debugging)
    - [Improving code quality](#improving-code-quality)

<!-- markdown-toc end -->

## Current feature set

* signed 32-bit integers (no other types yet)
* integer constants (decimal, hexadecimal, octal)
* logical negation (`!FOO` is 0 or 1)
* bitwise negation (`~FOO`)
* arithmetic negation (`-FOO` (unary subtraction))
* arithmetic subtraction (`foo - bar` (binary subtraction))
* arithmetic operators (`foo + bar`, `foo * bar`, `foo / bar`, `foo % bar`)
* bitwise operators ('foo & bar', 'foo | bar', 'foo ^ bar')
* arithmetic right shift (`foo >> bar`)
* arithmetic left shift (`foo << bar`)
* signed comparison (`foo < bar`, `foo <= bar`... '==', '!=', '>', '>=')
* boolean comparison '&&', '||'
* comments (`// foo \n` and `/* foo */`)
* sequences of statements (`foo; bar;`)
* return statements ('return foo;')
* nested blocks and local scopes for variables ('{ int i; { int j = i + 1; } }')
* if statements (`if (foo) { bar; }`) 
* if-else statements (`if (foo) { bar; } else { foobar; }`)
* labels and gotos (' goto err; ...... err: .... )
* local variables (`int` only)
* global variables (`int` only)
* static variables (`int` only)
* variable assignment (`int` only)
* sizeof (int `variable` only)
* while loops (`while (foo) { bar; }`)
* minimum of support for pointers on signed integers
* alloca() and arrays of signed integers
* function declaration (with multiple int arguments, returning int)
* function calls (with multiple int arguments, returning int)
* cdecl-type calling convention
* nested function calls
* return value from main() used as exit code
* microscopic stdlib support (write to stdout, read from stdin, assert, exit, rdtsc)
* simple preprocessor support (for now, we shell out to gcc -E)
* simplified peephole post-optimizer at instruction selection ('option -O1')
* few usual options from GCC/Clang command line ('-g' '-o' '-O0' '-O1' '-E' '-S' '-m32')

## License

GPL v2 license.

## Usage

You might need `clang`, `lex` and `yacc` installed. 'Flex' and GNU 'Gcc' and 'Bison' are known
to work, other yacc implementations may not.

Compiling babyc:

    # Compile the compiler.
    $ make

Usage:

    # Run it, producing a binary file.
    $ build/babyc -O1 -o a.out test_programs/immediate__return_1.c
    $ ./a.out
    $ echo $?

Viewing the code after preprocessing:

    $ build/babyc --dump-expansion test_programs/if_false__return_2.c

Viewing the AST:

    $ build/babyc --dump-ast test_programs/if_false__return_2.c

Generating and viewing the assembly:

    $ build/babyc -S test_programs/if_false__return_2.c
    $ cat out.s

Running tests:

    $ make test

### Debugging

If you want to debug a program that doesn't segfault, you can compile the program
with debugger support and set a breakpoint to any function:

    $ build/babyc -g test_programs/if_false__return_2.c
    $ gdb a.out
    (gdb) list main 
    (gdb) break main 
    (gdb) run
    
If you're debugging a compiled program that segfaults, you may want to
simply read/modify the out.s file, re-compile it, and then run it step by step
using GNU gdb, or more interactively, ddd (X11 wrapper over gdb)

    $ build/babyc -S test_programs/if_false__return_2.c
    $ vi out.s
      ... modify it
    $ as --32 -g -o out.o out.s
    $ ld -m elf_i386 -g -o a.out out.o
    $ gdb a.out
    (gdb) run
    ... it segfaults
    (gdb) layout asm
    ... shows which line the segfault occurred on
    (gdb) info registers
    ... shows the current state of the registers (`layout reg' also
    ... provides this data)

## Improving code quality

The make command will generate warnings, fix them. You can also run
with clang-analyzer to catch further issues:

    $ scan-build make

For code formatting, run:

    $ make format

## Known problems

* A few known memory leaks in compiler (symbol table in lexer, temp lists in AST build, ....).
* No syntax/semantic check, and no parsing recovery (use gcc to validate your test programs before using this compiler). Should use 'error' statement in .y file, and should pass line numbers from lex to yacc to alleviate that serious usability problem.
* Cannot parse standard C headers from '/usr/include' .... (see restrictions below)
* Operator precedence not fully tested, might not be fully compliant to C99 yet.
* Hardcoded temp filenames possibly left in current directory ('.extended.c', 'out.s' , 'out.o')

## Soon

* Working on a bit of code cleanup and code commenting, split large files.
* Adding source code line numbers in compiler error messages
* Adding a test_stdlib directory with more complex testcases, updating run_tests
* Support for "unsigned ints" and "uintxx_t"

## When to stop

* compile and run sha1 code
* compile and run primality test code
* support BF language (a real proof of Turing completeness)

## Not implemented

* No support for main parameters (argc, argv)
* No support for pointers (data pointers, function pointers ....)
* No support for '*' operator (address dereference)
* No support for '++', '--' pre/post-fix operators
* No support for '+=', '-=', '*=' unary operators
* No support for 'void' keyword
* No support for 'unsigned' type (only signed int)
* No support for 'signed' keyword (implicit signed int)
* No support for casts (only one type : signed int)
* No support for char, byte, bool, word, short, dword, long, longlong, qwords, intptr_t, uintxx_t types
* No support for nibble, tetrade, nybble, half-byte, semi-octet, quadbit types
* No support for float, double, long double types and IEEE 754  (only signed int)
* No support for arrays
* No support for enums, struct, typedefs ....
* No support for const, restrict, volatile, register keywords
* No support for inline keyword
* No support for loop variants ('for', 'do-while', 'foreach', ....)
* No support for break/continue statements
* No support for switch statements ('switch', 'case', 'default', 'duffs device'....)
* No support for strings and string operators ('strcpy', 'strcat', 'strcmp', ...)
* No support for printf(), gets()
* No support for memory allocation (malloc, realloc, free) 
* No support for threads and parallelization 
* No support for vectorization
* No support for signals and exceptions
* No support for self-modifiable code
* No support for asm, __as, __asm__ statements
* No support for GCC intrinsics
* No support for assembly file frontend
* No support for C++ file frontend
* No support for BASIC file frontend (why not ?)
* No support for BF file frontend (why not ?)
* No support for 64-bits architecture and processors other than i386
* No support for alien OSes (other than linux-like)
* No support for assembly annotation with source code references for debugger purposes
* optimizer: no AST coalescing
* optimizer: no optimizations specific to leaf functions
* optimizer: no register allocation/spill
* optimizer: no constant propagation
* optimizer: no loop unrolling
* optimizer: no function inlining
* optimizer: no peephole various simplifications
* optimizer: no dead code elimination
* optimizer: no red zone shortcuts
* optimizer: no optimizer at all .....



