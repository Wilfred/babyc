
/* ----------------------------------------------------------------
 *
 * Brave Algorithms Build Your Code
 *
 * ---------------------------------------------------------------- */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembly.h"

extern bool peephole_optimize;

static const int WORD_SIZE = 4;
static const int MAX_MNEMONIC_LENGTH = 7;

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

void write_unary_syntax(FILE *out, UnaryExpressionType unary_type,
                        Syntax *expression, Context *ctx) {
    if (peephole_optimize) {
        if (expression->type == IMMEDIATE) {
            int value = expression->immediate->value;

            if (unary_type == BITWISE_NEGATION) {
                value = ~value;
            } else if (unary_type == ARITHMETIC_NEGATION) {
                value = -value;
            } else if (unary_type == LOGICAL_NEGATION) {
                value = !value;
            }

            if (value) {
                emit_instr_format(out, "movl", "$%d, %%eax", value);
            } else {
                emit_instr(out, "xorl", "%eax, %eax");
            }
            return;
        }
    }

    write_syntax(out, expression, ctx);

    if (unary_type == BITWISE_NEGATION) {
        emit_instr(out, "notl", "%eax");
    } else if (unary_type == ARITHMETIC_NEGATION) {
        emit_instr(out, "negl", "%eax");
    } else if (unary_type == LOGICAL_NEGATION) {
        if (!syntax_is_boolean(expression)) {
            emit_instr(out, "testl", "$0xFFFFFFFF, %eax");
            emit_instr(out, "setz", "%al");
            emit_instr(out, "movzbl", "%al, %eax");
        } else {
            emit_instr(out, "xorl", "$1, %eax");
        }
    }
}
