
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
#include "log_error.h"

extern bool peephole_optimize;

/* -----------------------------------------------------------
 * Helper functions
 *
 * write binary operation
 * - evaluate left into the stack  offset[ebp]
 * - evaluate right into eax
 * - emit the binary opcode  op offset[ebp], eax
 *
 * write binary operation when right operand is immediate
 * - evaluate left into eax
 * - emit the binary opcode  op value, eax
 *
 * write binary operation when both operands are immediate
 * - evaluate the expression
 * - emit an opcode to set eax
 * ----------------------------------------------------------- */

ProcessorFlags write_binary_syntax(FILE *out, BinaryExpressionType binary_type,
                                   Syntax *left, Syntax *right, Context *ctx) {

    ProcessorFlags flag = FLAG_NONE;

    if (peephole_optimize) {

        /* swap operand if left operand is meediate and if swap makes sense */
        if (left->type == IMMEDIATE &&
            (binary_type == MULTIPLICATION || binary_type == OR ||
             binary_type == AND || binary_type == XOR ||
             binary_type == ADDITION || binary_type == EQUAL ||
             binary_type == NEQUAL || binary_type == LOGICAL_OR ||
             binary_type == LOGICAL_AND)) {
            Syntax *t = right;

            right = left;
            left = t;
        }

        /* detect right operand is immediate, generate short code */
        if (right->type == IMMEDIATE) {
            /* detect both operands are immediate, evaluate and generate short
             * code */
            if (left->type == IMMEDIATE) {
                AstInteger value;

                if (binary_type == LESS_THAN || binary_type == LARGER_THAN ||
                    binary_type == LESS_THAN_OR_EQUAL ||
                    binary_type == LARGER_THAN_OR_EQUAL ||
                    binary_type == EQUAL || binary_type == NEQUAL) {
                    ast_integer_binary_operation(
                        &value, &left->immediate->value,
                        &right->immediate->value, binary_type);
                    ast_integer_set_bool(&value, !ast_integer_is_zero(&value));
                } else if (binary_type == LOGICAL_OR) {
                    bool lvalue = !ast_integer_is_zero(&left->immediate->value);
                    bool rvalue =
                        !ast_integer_is_zero(&right->immediate->value);

                    ast_integer_set_bool(&value, lvalue || rvalue);
                } else if (binary_type == LOGICAL_AND) {
                    bool lvalue = !ast_integer_is_zero(&left->immediate->value);
                    bool rvalue =
                        !ast_integer_is_zero(&right->immediate->value);

                    ast_integer_set_bool(&value, lvalue && rvalue);
                } else {
                    ast_integer_binary_operation(
                        &value, &left->immediate->value,
                        &right->immediate->value, binary_type);
                }

                if (ast_integer_is_zero(&value)) {
                    emit_instr(out, "xorl", "%eax, %eax");
                    flag = FLAG_Z_VALID;
                } else if (ast_integer_is_one(&value)) {
                    emit_instr(out, "movl", "$1, %eax");
                    flag = FLAG_BOOL_VALID;
                } else {
                    emit_instr_format(out, "movl", "$%d, %%eax",
                                      ast_integer_get_int(&value));
                    flag = FLAG_NONE;
                }
            } else {
                flag = write_condition_syntax(out, left, ctx);

                if (binary_type == MULTIPLICATION) {
                    if (ast_integer_is_zero(&right->immediate->value)) {
                        emit_instr(out, "xorl", "%eax, %eax");
                        flag = FLAG_NZ_BOOL;
                    } else if (!ast_integer_is_one(&right->immediate->value)) {
                        emit_instr_format(
                            out, "imull", "$%d, %%eax",
                            ast_integer_get_int(&right->immediate->value));
                        flag = FLAG_NONE;
                    }
                } else if (binary_type == DIVISION) {
                    if (ast_integer_is_zero(&right->immediate->value)) {
                        log_error("Divide by 0");
                    }
                    if (!ast_integer_is_one(&right->immediate->value)) {
                        emit_instr_format(
                            out, "movl", "$%d, %%ecx",
                            ast_integer_get_int(&right->immediate->value));
                        emit_instr(out, "cltd", "");
                        emit_instr(out, "idivl", "%ecx");
                        flag = FLAG_NONE;
                    }
                } else if (binary_type == MODULUS) {
                    if (ast_integer_is_zero(&right->immediate->value)) {
                        log_error("Modulus by 0");
                    }
                    emit_instr_format(
                        out, "movl", "$%d, %%ecx",
                        ast_integer_get_int(&right->immediate->value));
                    emit_instr(out, "cltd", "");
                    emit_instr(out, "idivl", "%ecx");
                    emit_instr(out, "movl", "%edx, %eax");
                    flag = FLAG_NONE;
                } else if (binary_type == OR) {
                    unsigned value =
                        ast_integer_get_uint(&right->immediate->value);
                    if (value != 0) {
                        emit_instr_format(out, "orl", "$%u, %%eax", value);
                        flag = FLAG_Z_VALID;
                    }
                } else if (binary_type == AND) {
                    unsigned value =
                        ast_integer_get_uint(&right->immediate->value);
                    if (value == 0) {
                        emit_instr(out, "xorl", "%eax, %eax");
                        flag = FLAG_NZ_BOOL;
                    } else if (value == 1) {
                        emit_instr(out, "andl", "$1, %eax");
                        flag = FLAG_NZ_BOOL;
                    } else if (value != 0xffffffff) {
                        emit_instr_format(out, "andl", "$%u, %%eax", value);
                        flag = FLAG_Z_VALID;
                    }
                } else if (binary_type == XOR) {
                    unsigned value =
                        ast_integer_get_uint(&right->immediate->value);
                    if (value != 0) {
                        emit_instr_format(out, "xorl", "$%u, %%eax", value);
                        flag = FLAG_Z_VALID;
                    }
                } else if (binary_type == RSHIFT) {
                    unsigned value =
                        ast_integer_get_uint(&right->immediate->value);
                    if (value <= 31) {
                        if (value) {
                            emit_instr_format(out, "sarl", "$%d, %%eax", value);
                            flag = FLAG_Z_VALID;
                        }
                    } else {
                        emit_instr(out, "xorl", "%eax, %eax");
                        flag = FLAG_NZ_BOOL;
                    }
                } else if (binary_type == LSHIFT) {
                    unsigned value =
                        ast_integer_get_uint(&right->immediate->value);
                    if (value <= 31) {
                        if (value) {
                            emit_instr_format(out, "sall", "$%d, %%eax", value);
                            flag = FLAG_Z_VALID;
                        }
                    } else {
                        emit_instr(out, "xorl", "%eax, %eax");
                        flag = FLAG_NZ_BOOL;
                    }
                } else if (binary_type == ADDITION) {
                    unsigned value =
                        ast_integer_get_uint(&right->immediate->value);
                    if (value) {
                        emit_instr_format(out, "addl", "$%d, %%eax", value);
                        flag = FLAG_Z_VALID;
                    }
                } else if (binary_type == SUBTRACTION) {
                    unsigned value =
                        ast_integer_get_uint(&right->immediate->value);
                    if (value) {
                        emit_instr_format(out, "subl", "$%d, %%eax", value);
                        flag = FLAG_Z_VALID;
                    }
                } else if (binary_type == LESS_THAN ||
                           binary_type == LARGER_THAN ||
                           binary_type == LESS_THAN_OR_EQUAL ||
                           binary_type == LARGER_THAN_OR_EQUAL ||
                           binary_type == EQUAL || binary_type == NEQUAL) {
                    // To compare x < y in AT&T syntax, we write CMP y,x.
                    // http://stackoverflow.com/q/25493255/509706
                    unsigned value =
                        ast_integer_get_uint(&right->immediate->value);
                    emit_instr_format(out, "cmpl", "$%d, %%eax", value);
                    // Set the low byte of %eax to 0 or 1 depending on condition
                    // output
                    switch (binary_type) {
                    case LESS_THAN:
                        emit_instr(out, "setl", "%al");
                        flag = FLAG_BOOL_VALID;
                        break;
                    case LARGER_THAN:
                        emit_instr(out, "setg", "%al");
                        flag = FLAG_BOOL_VALID;
                        break;
                    case LESS_THAN_OR_EQUAL:
                        emit_instr(out, "setle", "%al");
                        flag = FLAG_BOOL_VALID;
                        break;
                    case LARGER_THAN_OR_EQUAL:
                        emit_instr(out, "setge", "%al");
                        flag = FLAG_BOOL_VALID;
                        break;
                    case EQUAL:
                        emit_instr(out, "setz", "%al");
                        flag = FLAG_Z_BOOL;
                        break;
                    case NEQUAL:
                        emit_instr(out, "setnz", "%al");
                        flag = FLAG_NZ_BOOL;
                        break;
                    default:
                        break;
                    }
                    // Zero the rest of %eax.
                    emit_instr(out, "movzbl", "%al, %eax");
                } else if (binary_type == LOGICAL_OR ||
                           binary_type == LOGICAL_AND) {
                    unsigned value =
                        ast_integer_get_uint(&right->immediate->value);
                    value = !!value;
                    if (flag == FLAG_Z_VALID) {
                        emit_instr(out, "setnz", "%al");
                        emit_instr(out, "movzbl", "%al, %eax");
                    } else if (flag == FLAG_Z_BOOL || flag == FLAG_NZ_BOOL ||
                               flag == FLAG_BOOL_VALID) {
                    } else if (!syntax_is_boolean(left)) {
                        emit_instr(out, "testl", "%eax, %eax");
                        emit_instr(out, "setnz", "%al");
                        emit_instr(out, "movzbl", "%al, %eax");
                    }
                    switch (binary_type) {
                    case LOGICAL_OR:
                        if (value)
                            emit_instr_format(out, "or", "$%d, %%eax", value);
                        break;
                    case LOGICAL_AND:
                        if (!value)
                            emit_instr_format(out, "and", "$%d, %%eax", value);
                        break;
                    default:
                        break;
                    }
                    flag = FLAG_NZ_BOOL;
                }
            }
            return flag;
        }
    }

    /* no immediate operand, push left evaluation on stack */
    int arg_size = syntax_type_size_value(left);

    ctx->offset -= arg_size;

    int stack_offset = ctx->offset;
    int left_flag = write_condition_syntax(out, left, ctx);

    emit_instr_format(out, "movl", "%%eax, %d(%%ebp)", stack_offset);
    int right_flag = write_condition_syntax(out, right, ctx);

    if (binary_type == MULTIPLICATION) {
        emit_instr_format(out, "imull", "%d(%%ebp), %%eax", stack_offset);
        flag = FLAG_NONE;
    } else if (binary_type == DIVISION) {
        emit_instr(out, "movl", "%eax, %ecx");
        emit_instr_format(out, "movl", "%d(%%ebp), %%eax", stack_offset);
        emit_instr(out, "cltd", "");
        emit_instr(out, "idivl", "%ecx");
        flag = FLAG_NONE;
    } else if (binary_type == MODULUS) {
        emit_instr(out, "movl", "%eax, %ecx");
        emit_instr_format(out, "movl", "%d(%%ebp), %%eax", stack_offset);
        emit_instr(out, "cltd", "");
        emit_instr(out, "idivl", "%ecx");
        emit_instr(out, "movl", "%edx, %eax");
        flag = FLAG_NONE;
    } else if (binary_type == OR) {
        emit_instr_format(out, "orl", "%d(%%ebp), %%eax", stack_offset);
        flag = FLAG_Z_VALID;
    } else if (binary_type == AND) {
        emit_instr_format(out, "andl", "%d(%%ebp), %%eax", stack_offset);
        flag = FLAG_Z_VALID;
    } else if (binary_type == XOR) {
        emit_instr_format(out, "xorl", "%d(%%ebp), %%eax", stack_offset);
        flag = FLAG_Z_VALID;
    } else if (binary_type == RSHIFT) {
        /* right shift, check for shift amount larger than operand size
         * if (shift > 31) dst = 0 else dest = eax >> shift
         */
        emit_instr(out, "movl", "%eax, %ecx");
        emit_instr_format(out, "movl", "%d(%%ebp), %%eax", stack_offset);
        emit_instr(out, "xor", "%edx, %edx");
        emit_instr(out, "sarl", "%cl, %eax");
        emit_instr(out, "cmpl", "$31, %ecx");
        emit_instr(out, "cmova", "%edx, %eax");
        flag = FLAG_NONE;
    } else if (binary_type == LSHIFT) {
        /* left shift, check for shift amount larger than operand size
         * if (shift > 31) dst = 0 else dest = eax << shift
         */
        emit_instr(out, "movl", "%eax, %ecx");
        emit_instr_format(out, "movl", "%d(%%ebp), %%eax", stack_offset);
        emit_instr(out, "xor", "%edx, %edx");
        emit_instr(out, "sall", "%cl, %eax");
        emit_instr(out, "cmpl", "$31, %ecx");
        emit_instr(out, "cmova", "%edx, %eax");
        flag = FLAG_NONE;
    } else if (binary_type == ADDITION) {
        emit_instr_format(out, "addl", "%d(%%ebp), %%eax", stack_offset);
        flag = FLAG_Z_VALID;
    } else if (binary_type == SUBTRACTION) {
        emit_instr_format(out, "subl", "%d(%%ebp), %%eax", stack_offset);
        emit_instr(out, "negl", "%eax");
        flag = FLAG_Z_VALID;
    } else if (binary_type == LESS_THAN || binary_type == LARGER_THAN ||
               binary_type == LESS_THAN_OR_EQUAL ||
               binary_type == LARGER_THAN_OR_EQUAL || binary_type == EQUAL ||
               binary_type == NEQUAL) {
        // To compare x < y in AT&T syntax, we write CMP y,x.
        // http://stackoverflow.com/q/25493255/509706
        emit_instr_format(out, "cmpl", "%%eax, %d(%%ebp)", stack_offset);
        // Set the low byte of %eax to 0 or 1 depending on condition output
        switch (binary_type) {
        case LESS_THAN:
            emit_instr(out, "setl", "%al");
            flag = FLAG_BOOL_VALID;
            break;
        case LARGER_THAN:
            emit_instr(out, "setg", "%al");
            flag = FLAG_BOOL_VALID;
            break;
        case LESS_THAN_OR_EQUAL:
            emit_instr(out, "setle", "%al");
            flag = FLAG_BOOL_VALID;
            break;
        case LARGER_THAN_OR_EQUAL:
            emit_instr(out, "setge", "%al");
            flag = FLAG_BOOL_VALID;
            break;
        case EQUAL:
            emit_instr(out, "setz", "%al");
            flag = FLAG_Z_BOOL;
            break;
        case NEQUAL:
            emit_instr(out, "setnz", "%al");
            flag = FLAG_NZ_BOOL;
            break;
        default:
            break;
        }
        // Zero the rest of %eax.
        emit_instr(out, "movzbl", "%al, %eax");
    } else if (binary_type == LOGICAL_OR || binary_type == LOGICAL_AND) {
        if (right_flag == FLAG_Z_VALID) {
            emit_instr(out, "setnz", "%al");
            emit_instr(out, "movzbl", "%al, %eax");
        } else if (right_flag == FLAG_Z_BOOL || right_flag == FLAG_NZ_BOOL ||
                   right_flag == FLAG_BOOL_VALID) {
        } else if (!syntax_is_boolean(right)) {
            emit_instr(out, "testl", "%eax, %eax");
            emit_instr(out, "setnz", "%al");
            emit_instr(out, "movzbl", "%al, %eax");
        }

        if (left_flag == FLAG_Z_BOOL || left_flag == FLAG_NZ_BOOL ||
            left_flag == FLAG_BOOL_VALID) {
            emit_instr_format(out, "movl", "%d(%%ebp), %%ecx", stack_offset);
        } else if (!syntax_is_boolean(left)) {
            emit_instr_format(out, "testl", "$0xFFFFFFFF, %d(%%ebp)",
                              stack_offset);
            emit_instr(out, "setnz", "%cl");
            emit_instr(out, "movzbl", "%cl, %ecx");
        } else {
            emit_instr_format(out, "movl", "%d(%%ebp), %%ecx", stack_offset);
        }
        switch (binary_type) {
        case LOGICAL_OR:
            emit_instr(out, "orl", "%ecx, %eax");
            break;
        case LOGICAL_AND:
            emit_instr(out, "andl", "%ecx, %eax");
            break;
        default:
            break;
        }
        flag = FLAG_NZ_BOOL;
    }
    ctx->offset += arg_size;
    return flag;
}
