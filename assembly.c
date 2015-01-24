#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <err.h>
#include "syntax.h"
#include "environment.h"
#include "context.h"

static const int WORD_SIZE = 4;
const int MAX_MNEMONIC_LENGTH = 7;

void emit_header(FILE *out, char *name) { fprintf(out, "%s\n", name); }

/* Write instruction INSTR with OPERANDS to OUT.
 *
 * Example:
 * emit_instr(out, "MOV", "%eax, 1");
 */
void emit_instr(FILE *out, char *instr, char *operands) {
    // TODO: fix duplication with emit_instr_format.
    // The assembler requires at least 4 spaces for indentation.
    fprintf(out, "    %s", instr);

    // Ensure our argument are aligned, regardless of the assembly
    // mnemonic length.
    int argument_offset = MAX_MNEMONIC_LENGTH - strlen(instr) + 4;
    while (argument_offset > 0) {
        fprintf(out, " ");
        argument_offset--;
    }

    fprintf(out, "%s\n", operands);
}

/* Write instruction INSTR with formatted operands OPERANDS_FORMAT to
 * OUT.
 *
 * Example:
 * emit_instr_format(out, "MOV", "%%eax, %s", 5);
 */
void emit_instr_format(FILE *out, char *instr, char *operands_format, ...) {
    // The assembler requires at least 4 spaces for indentation.
    fprintf(out, "    %s", instr);

    // Ensure our argument are aligned, regardless of the assembly
    // mnemonic length.
    int argument_offset = MAX_MNEMONIC_LENGTH - strlen(instr) + 4;
    while (argument_offset > 0) {
        fprintf(out, " ");
        argument_offset--;
    }

    va_list argptr;
    va_start(argptr, operands_format);
    vfprintf(out, operands_format, argptr);
    va_end(argptr);

    fputs("\n", out);
}

char *fresh_local_label(char *prefix, Context *ctx) {
    // We assume we never write more than 6 chars of digits, plus a '.' and '_'.
    size_t buffer_size = strlen(prefix) + 8;
    char *buffer = malloc(buffer_size);

    snprintf(buffer, buffer_size, ".%s_%d", prefix, ctx->label_count);
    ctx->label_count++;

    return buffer;
}

void emit_label(FILE *out, char *label) { fprintf(out, "%s:\n", label); }

void emit_function_declaration(FILE *out, char *name) {
    fprintf(out, "    .global %s\n", name);
    fprintf(out, "%s:\n", name);
}

void emit_function_prologue(FILE *out) {
    emit_instr(out, "pushl", "%ebp");
    emit_instr(out, "mov", "%esp, %ebp");
    fprintf(out, "\n");
}

void emit_return(FILE *out) {
    fprintf(out, "    leave\n");
    fprintf(out, "    ret\n");
}

void emit_function_epilogue(FILE *out) {
    emit_return(out);
    fprintf(out, "\n");
}

void write_header(FILE *out) { emit_header(out, "    .text"); }

void write_footer(FILE *out) {
    // TODO: this will break if a user defines a function called '_start'.
    emit_function_declaration(out, "_start");
    emit_function_prologue(out);
    emit_instr(out, "call", "main");
    emit_instr(out, "mov", "%eax, %ebx");
    emit_instr(out, "mov", "$1, %eax");
    emit_instr(out, "int", "$0x80");
}

void write_syntax(FILE *out, Syntax *syntax, Context *ctx) {
    // Note stack_offset is the next unused memory address in the
    // stack, so we can use it directly but must adjust it for the next caller.
    if (syntax->type == UNARY_OPERATOR) {
        UnaryExpression *unary_syntax = syntax->unary_expression;

        write_syntax(out, unary_syntax->expression, ctx);

        if (unary_syntax->unary_type == BITWISE_NEGATION) {
            emit_instr(out, "not", "%eax");
        } else {
            emit_instr(out, "test", "$0xFFFFFFFF, %eax");
            emit_instr(out, "setz", "%al");
        }
    } else if (syntax->type == IMMEDIATE) {
        emit_instr_format(out, "mov", "$%d, %%eax", syntax->immediate->value);

    } else if (syntax->type == VARIABLE) {
        emit_instr_format(
            out, "mov", "%d(%%ebp), %%eax",
            environment_get_offset(ctx->env, syntax->variable->var_name));

    } else if (syntax->type == BINARY_OPERATOR) {
        BinaryExpression *binary_syntax = syntax->binary_expression;
        int stack_offset = ctx->stack_offset;
        ctx->stack_offset -= WORD_SIZE;

        emit_instr(out, "sub", "$4, %esp");
        write_syntax(out, binary_syntax->left, ctx);
        emit_instr_format(out, "mov", "%%eax, %d(%%ebp)", stack_offset);

        write_syntax(out, binary_syntax->right, ctx);

        if (binary_syntax->binary_type == MULTIPLICATION) {
            emit_instr_format(out, "mull", "%d(%%ebp)", stack_offset);

        } else if (binary_syntax->binary_type == ADDITION) {
            emit_instr_format(out, "add", "%d(%%ebp), %%eax", stack_offset);

        } else if (binary_syntax->binary_type == SUBTRACTION) {
            emit_instr_format(out, "sub", "%%eax, %d(%%ebp)", stack_offset);
            emit_instr_format(out, "mov", "%d(%%ebp), %%eax", stack_offset);

        } else if (binary_syntax->binary_type == LESS_THAN) {
            // To compare x < y in AT&T syntax, we write CMP y,x.
            // http://stackoverflow.com/q/25493255/509706
            emit_instr_format(out, "cmp", "%%eax, %d(%%ebp)", stack_offset);
            // Set the low byte of %eax to 0 or 1 depending on whether
            // it was less than.
            emit_instr(out, "setl", "%al");
            // Zero the rest of %eax.
            emit_instr(out, "movzbl", "%al, %eax");

        } else if (binary_syntax->binary_type == LESS_THAN_OR_EQUAL) {
            // To compare x < y in AT&T syntax, we write CMP y,x.
            // http://stackoverflow.com/q/25493255/509706
            emit_instr_format(out, "cmp", "%%eax, %d(%%ebp)", stack_offset);
            // Set the low byte of %eax to 0 or 1 depending on whether
            // it was less than or equal.
            emit_instr(out, "setle", "%al");
            // Zero the rest of %eax.
            emit_instr(out, "movzbl", "%al, %eax");
        }

    } else if (syntax->type == ASSIGNMENT) {
        write_syntax(out, syntax->assignment->expression, ctx);

        emit_instr_format(
            out, "mov", "%%eax, %d(%%ebp)",
            environment_get_offset(ctx->env, syntax->variable->var_name));

    } else if (syntax->type == RETURN_STATEMENT) {
        ReturnStatement *return_statement = syntax->return_statement;
        write_syntax(out, return_statement->expression, ctx);

        emit_return(out);

    } else if (syntax->type == FUNCTION_CALL) {
        emit_instr_format(out, "call", syntax->function_call->function_name);

    } else if (syntax->type == IF_STATEMENT) {
        IfStatement *if_statement = syntax->if_statement;
        write_syntax(out, if_statement->condition, ctx);

        char *label = fresh_local_label("if_end", ctx);

        emit_instr(out, "test", "%eax, %eax");
        emit_instr_format(out, "jz", "%s", label);

        write_syntax(out, if_statement->then, ctx);
        emit_label(out, label);

    } else if (syntax->type == WHILE_SYNTAX) {
        WhileStatement *while_statement = syntax->while_statement;

        char *start_label = fresh_local_label("while_start", ctx);
        char *end_label = fresh_local_label("while_end", ctx);

        emit_label(out, start_label);
        write_syntax(out, while_statement->condition, ctx);

        emit_instr(out, "test", "%eax, %eax");
        emit_instr_format(out, "jz", "%s", end_label);

        write_syntax(out, while_statement->body, ctx);
        emit_instr_format(out, "jmp", "%s", start_label);
        emit_label(out, end_label);

    } else if (syntax->type == DEFINE_VAR) {
        DefineVarStatement *define_var_statement = syntax->define_var_statement;
        int stack_offset = ctx->stack_offset;

        environment_set_offset(ctx->env, define_var_statement->var_name,
                               stack_offset);
        emit_instr(out, "sub", "$4, %esp");

        ctx->stack_offset -= WORD_SIZE;
        write_syntax(out, define_var_statement->init_value, ctx);
        emit_instr_format(out, "mov", "%%eax, %d(%%ebp)\n", stack_offset);

    } else if (syntax->type == BLOCK) {
        List *statements = syntax->block->statements;
        for (int i = 0; i < list_length(statements); i++) {
            write_syntax(out, list_get(statements, i), ctx);
        }
    } else if (syntax->type == FUNCTION) {
        new_scope(ctx);

        emit_function_declaration(out, syntax->function->name);
        emit_function_prologue(out);
        write_syntax(out, syntax->function->root_block, ctx);
        emit_function_epilogue(out);

    } else if (syntax->type == TOP_LEVEL) {
        // TODO: treat the 'main' function specially.
        List *declarations = syntax->top_level->declarations;
        for (int i = 0; i < list_length(declarations); i++) {
            write_syntax(out, list_get(declarations, i), ctx);
        }

    } else {
        warnx("Unknown syntax %s", syntax_type_name(syntax));
        assert(false);
    }
}

void write_assembly(Syntax *syntax) {
    FILE *out = fopen("out.s", "wb");

    write_header(out);

    Context *ctx = new_context();

    write_syntax(out, syntax, ctx);
    write_footer(out);

    context_free(ctx);
    fclose(out);
}
