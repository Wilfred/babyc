
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
 * write expression to be used in conditional statement
 *
 * ----------------------------------------------------------- */

ProcessorFlags write_condition_syntax(FILE *out, Syntax *condition,
                                      Context *ctx) {
    ProcessorFlags flag = FLAG_NONE;
    if (peephole_optimize) {
        if (condition->type == UNARY_OPERATOR) {
            UnaryExpression *unary_syntax = condition->unary_expression;
            flag = write_unary_syntax(out, unary_syntax->unary_type,
                                      unary_syntax->expression, ctx);
            return flag;
        } else if (condition->type == BINARY_OPERATOR) {
            BinaryExpression *binary_syntax = condition->binary_expression;
            flag = write_binary_syntax(out, binary_syntax->binary_type,
                                       binary_syntax->left,
                                       binary_syntax->right, ctx);
            return flag;
        }
    }

    write_syntax(out, condition, ctx);
    return flag;
}
