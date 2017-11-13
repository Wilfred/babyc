
/* ----------------------------------------------------------------
 *
 * Brave Algorithms Build Your Code
 *
 * ---------------------------------------------------------------- */

#ifndef BABYC_ASSEMBLY_HEADER
#define BABYC_ASSEMBLY_HEADER

#include "syntax.h"
#include <stdio.h>

void write_all_syntax(FILE *out, Syntax *syntax);

void write_assembly(char *file_name, Syntax *syntax);

typedef struct Context {
    int label_count;
    int while_label;
    int offset;
    int max_offset;
    char *function_end_label;
} Context;

void write_binary_syntax(FILE *out, BinaryExpressionType binary_type,
                         Syntax *left, Syntax *right, Context *ctx);

void write_unary_syntax(FILE *out, UnaryExpressionType unary_type,
                        Syntax *expression, Context *ctx);

void emit_instr_format(FILE *out, char *instr, char *operands_format, ...);

void emit_instr(FILE *out, char *instr, char *operands);

void write_syntax(FILE *out, Syntax *syntax, Context *ctx);

#endif
