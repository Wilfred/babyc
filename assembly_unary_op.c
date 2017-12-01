
/* ----------------------------------------------------------------
 *
 * BabyC Toy compiler for educational purposes
 *
 * ---------------------------------------------------------------- */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast_annotate.h"
#include "assembly.h"

extern bool peephole_optimize;

/* -----------------------------------------------------------
 * Helper functions
 *
 * write unary operation when operand is an expression
 * - evaluate expression into eax
 * - emit the binary opcode  op value, eax
 *
 * write binary operation when operand is immediate
 * - evaluate the expression
 * - emit an opcode to set eax
 * ----------------------------------------------------------- */

ProcessorFlags write_unary_syntax(FILE *out, UnaryExpressionType unary_type,
                                  Syntax *expression, Context *ctx) {
    ProcessorFlags flag = FLAG_NONE;

    if (peephole_optimize) {
        if (expression->type == IMMEDIATE) {
            AstInteger value;
            ast_integer_unary_operation(&value, &expression->immediate->value,
                                        unary_type);

            if (ast_integer_is_zero(&value)) {
                emit_instr(out, "xorl", "%eax, %eax");
                flag = FLAG_Z_VALID;
            } else if (ast_integer_is_one(&value)) {
                emit_instr(out, "movl", "$1, %eax");
                flag = FLAG_BOOL_VALID;
            } else {
                unsigned v = ast_integer_get_uint(&value);
                emit_instr_format(out, "movl", "$%u, %%eax", v);
            }
            return flag;
        }
    }

    flag = write_condition_syntax(out, expression, ctx);

    if (unary_type == CAST) {
        // TODO
    } else if (unary_type == BITWISE_NEGATION) {
        emit_instr(out, "notl", "%eax");
        flag = FLAG_Z_VALID;
    } else if (unary_type == ARITHMETIC_NEGATION) {
        emit_instr(out, "negl", "%eax");
        flag = FLAG_Z_VALID;
    } else if (unary_type == LOGICAL_NEGATION) {
        if (flag == FLAG_Z_VALID) {
            emit_instr(out, "testl", "%eax, %eax");
            emit_instr(out, "setz", "%al");
            emit_instr(out, "movzbl", "%al, %eax");
            flag = FLAG_Z_BOOL;
        } else if (flag == FLAG_Z_BOOL) {
            emit_instr(out, "xorl", "$1, %eax");
            flag = FLAG_Z_BOOL;
        } else if (flag == FLAG_NZ_BOOL) {
            emit_instr(out, "xorl", "$1, %eax");
            flag = FLAG_NZ_BOOL;
        } else if (flag == FLAG_BOOL_VALID) {
            emit_instr(out, "xorl", "$1, %eax");
            flag = FLAG_Z_BOOL;
        } else if (!syntax_is_boolean(expression)) {
            emit_instr(out, "testl", "%eax, %eax");
            emit_instr(out, "setz", "%al");
            emit_instr(out, "movzbl", "%al, %eax");
            flag = FLAG_Z_BOOL;
        } else {
            emit_instr(out, "xorl", "$1, %eax");
            flag = FLAG_Z_BOOL;
        }
    }
    return flag;
}
