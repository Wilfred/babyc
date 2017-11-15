
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
#include "log_error.h"

extern bool peephole_optimize;

static const int WORD_SIZE = 4;

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
                int value = 0;
                int lvalue = left->immediate->value;
                int rvalue = right->immediate->value;

                if (binary_type == MULTIPLICATION) {
                    value = lvalue * rvalue;
                } else if (binary_type == DIVISION) {
                    if (rvalue == 0)
                        log_error("Divide by zero !\n");
                    value = lvalue / rvalue;
                } else if (binary_type == MODULUS) {
                    if (rvalue == 0)
                        log_error("Modulus is zero !\n");
                    value = lvalue % rvalue;
                } else if (binary_type == OR) {
                    value = lvalue | rvalue;
                } else if (binary_type == AND) {
                    value = lvalue & rvalue;
                } else if (binary_type == XOR) {
                    value = lvalue ^ rvalue;
                } else if (binary_type == RSHIFT) {
                    value = rvalue <= 31 ? lvalue >> rvalue : 0;
                } else if (binary_type == LSHIFT) {
                    value = rvalue <= 31 ? lvalue << rvalue : 0;
                } else if (binary_type == ADDITION) {
                    value = lvalue + rvalue;
                } else if (binary_type == SUBTRACTION) {
                    value = lvalue - rvalue;
                } else if (binary_type == LESS_THAN ||
                           binary_type == LARGER_THAN ||
                           binary_type == LESS_THAN_OR_EQUAL ||
                           binary_type == LARGER_THAN_OR_EQUAL ||
                           binary_type == EQUAL || binary_type == NEQUAL ||
                           binary_type == LOGICAL_OR ||
                           binary_type == LOGICAL_AND) {
                    bool b = 0;

                    switch (binary_type) {
                    case LESS_THAN:
                        b = (lvalue < rvalue);
                        break;
                    case LARGER_THAN:
                        b = (lvalue > rvalue);
                        break;
                    case LESS_THAN_OR_EQUAL:
                        b = (lvalue <= rvalue);
                        break;
                    case LARGER_THAN_OR_EQUAL:
                        b = (lvalue >= rvalue);
                        break;
                    case EQUAL:
                        b = (lvalue == rvalue);
                        break;
                    case NEQUAL:
                        b = (lvalue != rvalue);
                        break;
                    case LOGICAL_OR:
                        b = (!!lvalue || !!rvalue);
                        break;
                    case LOGICAL_AND:
                        b = (!!lvalue && !!rvalue);
                        break;
                    default:
                        break;
                    }
                    value = b;
                    flag = FLAG_BOOL_VALID;
                }
                if (value == 0) {
                    emit_instr(out, "xorl", "%eax, %eax");
                    flag = FLAG_Z_VALID;
                } else if (value == 1) {
                    emit_instr(out, "movl", "$1, %eax");
                    flag = FLAG_BOOL_VALID;
                } else {
                    emit_instr_format(out, "movl", "$%d, %%eax", value);
                    flag = FLAG_NONE;
                }
            } else {
                int value = right->immediate->value;

                flag = write_condition_syntax(out, left, ctx);

                if (binary_type == MULTIPLICATION) {
                    if (value == 0) {
                        emit_instr(out, "xorl", "%eax, %eax");
                        flag = FLAG_Z_VALID;
                    } else if (value == -1) {
                        emit_instr(out, "negl", "%eax");
                        flag = FLAG_Z_VALID;
                    } else if (value != 1) {
                        emit_instr_format(out, "imull", "$%d, %%eax", value);
                        flag = FLAG_NONE;
                    }
                } else if (binary_type == DIVISION) {
                    if (value == -1) {
                        emit_instr(out, "negl", "%eax");
                        flag = FLAG_Z_VALID;
                    } else if (value != 1) {
                        emit_instr_format(out, "movl", "$%d, %%ecx", value);
                        emit_instr(out, "xorl", "%edx, %edx");
                        emit_instr(out, "idivl", "%ecx");
                        flag = FLAG_NONE;
                    }
                } else if (binary_type == MODULUS) {
                    emit_instr_format(out, "movl", "$%d, %%ecx", value);
                    emit_instr(out, "xorl", "%edx, %edx");
                    emit_instr(out, "idivl", "%ecx");
                    emit_instr(out, "movl", "%edx, %eax");
                    flag = FLAG_NONE;
                } else if (binary_type == OR) {
                    if (value != 0) {
                        emit_instr_format(out, "orl", "$%d, %%eax", value);
                        flag = FLAG_Z_VALID;
                    }
                } else if (binary_type == AND) {
                    if (value != -1) {
                        emit_instr_format(out, "andl", "$%d, %%eax", value);
                        flag = FLAG_Z_VALID;
                    }
                } else if (binary_type == XOR) {
                    if (value != 0) {
                        emit_instr_format(out, "xorl", "$%d, %%eax", value);
                        flag = FLAG_Z_VALID;
                    }
                } else if (binary_type == RSHIFT) {
                    if (value <= 31) {
                        if (value) {
                            emit_instr_format(out, "sarl", "$%d, %%eax", value);
                            flag = FLAG_Z_VALID;
                        }
                    } else {
                        emit_instr(out, "xorl", "%eax, %eax");
                        flag = FLAG_Z_VALID;
                    }
                } else if (binary_type == LSHIFT) {
                    if (value <= 31) {
                        if (value) {
                            emit_instr_format(out, "sall", "$%d, %%eax", value);
                            flag = FLAG_Z_VALID;
                        }
                    } else {
                        emit_instr(out, "xorl", "%eax, %eax");
                        flag = FLAG_Z_VALID;
                    }
                } else if (binary_type == ADDITION) {
                    if (value) {
                        emit_instr_format(out, "addl", "$%d, %%eax", value);
                        flag = FLAG_Z_VALID;
                    }
                } else if (binary_type == SUBTRACTION) {
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
    ctx->offset -= WORD_SIZE;

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
        emit_instr(out, "xorl", "%edx, %edx");
        emit_instr(out, "idivl", "%ecx");
        flag = FLAG_NONE;
    } else if (binary_type == MODULUS) {
        emit_instr(out, "movl", "%eax, %ecx");
        emit_instr_format(out, "movl", "%d(%%ebp), %%eax", stack_offset);
        emit_instr(out, "xorl", "%edx, %edx");
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
        emit_instr(out, "movl", "%eax, %ecx");
        emit_instr_format(out, "movl", "%d(%%ebp), %%eax", stack_offset);
        emit_instr(out, "subl", "%ecx, %eax");
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
    ctx->offset += WORD_SIZE;
    return flag;
}
