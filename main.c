
/* ----------------------------------------------------------------
 *
 * Brave Algorithms Build Your Code
 *
 * ---------------------------------------------------------------- */

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "assembly.h"
#include "syntax.h"

void print_help() {
    printf("Babyc is a very basic C compiler.\n\n");
    printf("To compile a file into a.out:\n");
    printf("    $ babyc foo.c\n");
    printf("To compile a file into foo.exe:\n");
    printf("    $ babyc -o foo.exe foo.c\n");
    printf("To output the preprocessed code without parsing:\n");
    printf("    $ babyc -E foo.c\n");
    printf("To output the AST without compiling:\n");
    printf("    $ babyc --dump-ast foo.c\n");
    printf("To generate the assembly file out.s:\n");
    printf("    $ babyc -S foo.c\n");
    printf("To print this message:\n");
    printf("    $ babyc --help\n\n");
    printf("For more information, see https://github.com/Wilfred/babyc\n");
}

extern int yyparse(void);

typedef enum {
    MACRO_EXPAND,
    PARSE,
    EMIT_ASM,
    BUILD_EXE,
} stage_t;

int main(int argc, char *argv[]) {
    stage_t terminate_at = BUILD_EXE;
    char *input_file_name = NULL;
    char *output_file_name = NULL;
    int ret = 1;
    int debug = 0;

    for (int i = 1; i < argc; i++) {

        /* GCC-like  options */
        if (!strcmp(argv[i], "--help")) {
            print_help();
            return 0;
        }
        if (!strcmp(argv[i], "--version")) {
            printf("Babyc 1.0.0\n");
            return 0;
        }
        if (!strcmp(argv[i], "-E")) {
            terminate_at = MACRO_EXPAND;
            continue;
        }
        if (!strcmp(argv[i], "-S")) {
            terminate_at = EMIT_ASM;
            continue;
        }
        if (!strcmp(argv[i], "-g")) {
            debug = 1;
            continue;
        }

        /* GCC-like implicit options (what is really implemented) */
        if (!strcmp(argv[i], "-m32")) {
            continue;
        }
        if (!strcmp(argv[i], "-O0")) {
            continue;
        }
        if (!strcmp(argv[i], "-fomit-frame-pointer")) {
            continue;
        }
        if (!strcmp(argv[i], "-fno-stack-protector")) {
            continue;
        }
        if (!strcmp(argv[i], "-Wl,--stack,4096")) {
            continue;
        }

        /* historical options */
        if (!strcmp(argv[i], "--dump-ast")) {
            terminate_at = PARSE;
            continue;
        }
        if (!strcmp(argv[i], "--dump-expansion")) {
            terminate_at = MACRO_EXPAND;
            continue;
        }

        /* one output file name */
        if (!strcmp(argv[i], "-o")) {
            if (output_file_name) {
                printf("Too many output files\n");
                print_help();
                return 1;
            }
            output_file_name = argv[++i];
            continue;
        }

        /* one input file name */
        if (input_file_name) {
            printf("Too many input files\n");
            print_help();
            return 1;
        }
        input_file_name = argv[i];
    }

    if (!input_file_name) {
        printf("Input file missing\n");
        print_help();
        return 1;
    }
    if (!output_file_name) {
        output_file_name = "a.out";
    }

    int result;

    /* -------------------------------------------------------------------
     *  Preprocessor stage
     * ------------------------------------------------------------------- */
    // TODO: create a proper temporary file from the preprocessor.
    char command[1024] = {0};
    snprintf(command, 1024, "gcc -E %s > .expanded.c", input_file_name);
    result = system(command);
    if (result != 0) {
        puts("Macro expansion failed!");
        goto cleanup_cpp_file;
    }

    if (terminate_at == MACRO_EXPAND) {
        int c;

        FILE *f = fopen(".expanded.c", "r");

        while ((c = getc(f)) != EOF) {
            putchar(c);
        }
        fclose(f);
        ret = 0;
        goto cleanup_cpp_file;
    }

    /* -------------------------------------------------------------------
     *  Parsing stage (Build AST)
     * ------------------------------------------------------------------- */

    parser_setup(".expanded.c");

    result = yyparse();

    Syntax *complete_syntax = parser_complete();

    if (result != 0) {
        printf("Parser error occurred\n");
        goto cleanup_cpp_file;
    }

    if (terminate_at == PARSE) {

        /* -------------------------------------------------------------------
         *  Debug (dump AST)
         * -------------------------------------------------------------------
         */

        print_syntax_item(complete_syntax);
        syntax_free(complete_syntax);
        goto cleanup_cpp_file;
    }

    /* -------------------------------------------------------------------
     *  Assembly generation (write AST)
     * ------------------------------------------------------------------- */

    write_assembly("out.s", complete_syntax);
    syntax_free(complete_syntax);

    if (terminate_at == EMIT_ASM) {
        printf("Written out.s\n");
        printf("Build it with:\n");
        printf("    $ as --32 -g -o out.o out.s\n");
        printf("    $ ld -m elf_i386 -g -o a.out out.o\n");
        printf("Debug it with:\n");
        printf("    $ objdump -d a.out\n");
        printf("    $ gdb a.out\n");
        goto cleanup_cpp_file;
    }

    /* -------------------------------------------------------------------
     *  Assembler stage
     * ------------------------------------------------------------------- */

    snprintf(command, 1024, "as --32 %s -o out.o out.s", debug ? "-g" : "");
    result = system(command);
    if (result != 0) {
        printf("Assembler error occurred\n");
        goto cleanup_s_file;
    }

    /* -------------------------------------------------------------------
     *  Linker stage
     * ------------------------------------------------------------------- */

    // TODO : verify if the ELF stack setup comes from
    // - ulimit (this is now the trend)
    // - setrlimit
    // - ld command line
    snprintf(command, 1024,
             "ld -m elf_i386 -e _start -z stack-size=4096 %s -o %s out.o",
             debug ? "-g" : "-s", output_file_name);
    result = system(command);
    if (result != 0) {
        printf("Linker error occurred\n");
        goto cleanup_o_file;
    }

    ret = 0;

cleanup_o_file:

    if (!debug)
        unlink("out.o");

cleanup_s_file:

    if (!debug)
        unlink("out.s");

cleanup_cpp_file:

    unlink(".expanded.c");

    return ret;
}
