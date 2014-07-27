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
    fprintf(out, "    mov     %%eax, %%ebx\n");
    fprintf(out, "    movl    $1, %%eax\n");
    fprintf(out, "    int     $0x80\n");
}

void write_syntax(FILE *out, Syntax *syntax) {
    if (syntax->type == UNARY_OPERATOR) {
        UnarySyntax *unary_syntax = syntax->unary_expression;

        write_syntax(out, unary_syntax->expression);

        if (unary_syntax->unary_type == BITWISE_NEGATION) {
            fprintf(out, "    not     %%eax\n");
        } else {
            fprintf(out, "    test    $0xFFFFFFFF, %%eax\n");
            fprintf(out, "    setz    %%al\n");
        }
    } else if (syntax->type == IMMEDIATE) {
        fprintf(out, "    movl    $%d, %%eax\n", syntax->value);
    } else if (syntax->type == BINARY_OPERATOR) {
        BinarySyntax *binary_syntax = syntax->binary_expression;

        // TODO: we should keep track of local intermediates instead
        // of always writing to the same place in memory.
        write_syntax(out, binary_syntax->left);
        fprintf(out, "    mov     %%eax, -0x4(%%ebp)\n");
        write_syntax(out, binary_syntax->right);
        fprintf(out, "    add     -0x4(%%ebp), %%eax\n");
    }
}

void write_assembly(Syntax *syntax) {
    FILE *out = fopen("out.s", "wb");

    write_header(out);
    write_syntax(out, syntax);
    write_footer(out);
    
    fclose(out);
}
