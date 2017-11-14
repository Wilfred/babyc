
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

typedef enum {
    FLAG_NONE = 0x00,
    FLAG_BOOL_VALID = 0x01,
    FLAG_Z_VALID = 0x02,
    FLAG_Z_BOOL = 0x04,
    FLAG_NZ_BOOL = 0x8,
} ProcessorFlags;

ProcessorFlags write_binary_syntax(FILE *out, BinaryExpressionType binary_type,
                                   Syntax *left, Syntax *right, Context *ctx);

ProcessorFlags write_unary_syntax(FILE *out, UnaryExpressionType unary_type,
                                  Syntax *expression, Context *ctx);

ProcessorFlags write_condition_syntax(FILE *out, Syntax *expression,
                                      Context *ctx);

void emit_instr_format(FILE *out, char *instr, char *operands_format, ...);

void emit_instr(FILE *out, char *instr, char *operands);

void write_syntax(FILE *out, Syntax *syntax, Context *ctx);

#endif
