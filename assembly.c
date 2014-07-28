#include <stdio.h>
#include "syntax.h"

void emit_header(FILE *out, char *name) {
    // The assembler requries at least 4 spaces for indentation.
    fprintf(out, "%s\n", name);
}

void emit_insn(FILE *out, char *insn) {
    // The assembler requries at least 4 spaces for indentation.
    fprintf(out, "    %s\n", insn);
}

void write_header(FILE *out) {
    emit_header(out, ".text");
    emit_insn(out, ".global _start\n");
    emit_header(out, "_start:");
    emit_insn(out, "mov     %esp, %ebp");
    emit_header(out, "");
}

void write_footer(FILE *out) {
    emit_header(out, "");
    emit_insn(out, "mov     %eax, %ebx");
    emit_insn(out, "mov     $1, %eax");
    emit_insn(out, "int     $0x80");
}

void write_syntax(FILE *out, Syntax *syntax) {
    if (syntax->type == UNARY_OPERATOR) {
        UnarySyntax *unary_syntax = syntax->unary_expression;

        write_syntax(out, unary_syntax->expression);

        if (unary_syntax->unary_type == BITWISE_NEGATION) {
            emit_insn(out, "not     %eax");
        } else {
            emit_insn(out, "test    $0xFFFFFFFF, %eax");
            emit_insn(out, "setz    %al");
        }
    } else if (syntax->type == IMMEDIATE) {
        fprintf(out, "    mov     $%d, %%eax\n", syntax->value);
    } else if (syntax->type == BINARY_OPERATOR) {
        BinarySyntax *binary_syntax = syntax->binary_expression;

        write_syntax(out, binary_syntax->left);
        emit_insn(out, "mov     %eax, -0x4(%ebp)");
        write_syntax(out, binary_syntax->right);
        emit_insn(out, "add     -0x4(%ebp), %eax");
    }
}

void write_assembly(Syntax *syntax) {
    FILE *out = fopen("out.s", "wb");

    write_header(out);
    write_syntax(out, syntax);
    write_footer(out);
    
    fclose(out);
}
