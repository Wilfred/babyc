#include <stdio.h>
#include "syntax.c"

void write_header(FILE *out) {
    fprintf(out, ".text\n");
    // We seem to require at least 8 spaces for indentation.
    fprintf(out, "    .global _start\n\n");
    fprintf(out, "_start:\n");
    fprintf(out, "    movl    %%esp, %%ebp\n");
    fprintf(out, "    subl    $4, %%esp\n");
    fprintf(out, "\n");
}

void write_footer(FILE *out) {
    fprintf(out, "    movl    $1, %%eax\n");
    fprintf(out, "    int     $0x80\n");
}

void write_syntax(FILE *out, Syntax *syntax) {
    // TODO: do everything in eax, then move to ebx for exit.
    if (syntax->type == UNARY_OPERATOR) {
        UnarySyntax *unary_syntax = syntax->unary_expression;

        write_syntax(out, unary_syntax->expression);

        if (unary_syntax->unary_type == BITWISE_NEGATION) {
            fprintf(out, "    not     %%ebx\n");
        } else {
            fprintf(out, "    test    $0xFFFFFFFF, %%ebx\n");
            fprintf(out, "    setz    %%bl\n");
        }
    } else if (syntax->type == IMMEDIATE) {
        fprintf(out, "    movl    $%d, %%ebx\n", syntax->value);
    } else if (syntax->type == BINARY_OPERATOR) {
        BinarySyntax *binary_syntax = syntax->binary_expression;

        // TODO: we should keep track of local intermediates instead
        // of always writing to the same place in memory.
        write_syntax(out, binary_syntax->left);
        fprintf(out, "    mov     %%ebx, -0x4(%%ebp)\n");
        write_syntax(out, binary_syntax->right);
        fprintf(out, "    add     -0x4(%%ebp), %%ebx\n");
    }
}

void write_assembly(Syntax *syntax) {
    FILE *out = fopen("out.s", "wb");

    write_header(out);
    write_syntax(out, syntax);
    write_footer(out);
    
    fclose(out);
}
