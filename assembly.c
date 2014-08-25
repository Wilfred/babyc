#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <err.h>
#include "syntax.h"
#include "environment.h"

#define WORD_SIZE 4

void emit_header(FILE *out, char *name) {
    // The assembler requries at least 4 spaces for indentation.
    fprintf(out, "%s\n", name);
}

void emit_insn(FILE *out, char *insn) {
    // The assembler requries at least 4 spaces for indentation.
    fprintf(out, "    %s\n", insn);
}

int label_count = 0;

char *fresh_local_label(char *prefix) {
    // We assume we never write more than 6 chars of digits, plus a '.' and '_'.
    size_t buffer_size = strlen(prefix) + 8;
    char *buffer = malloc(buffer_size);

    snprintf(buffer, buffer_size, ".%s_%d", prefix, label_count);
    label_count++;

    return buffer;
}

void emit_label(FILE *out, char *label) {
    fprintf(out, "%s:\n", label);
}

void write_header(FILE *out) {
    emit_header(out, ".text");
    emit_insn(out, ".global _start\n");
    emit_header(out, "_start:");
    emit_insn(out, "mov     %esp, %ebp");
    emit_header(out, "");
}

typedef struct Context {
    int *stack_offset;
    Environment *env;
} Context;

void write_syntax(FILE *out, Syntax *syntax, Context *ctx) {
    // Note stack_offset is the next unused memory address in the
    // stack, so we can use it directly but must adjust it for the next caller.
    if (syntax->type == UNARY_OPERATOR) {
        UnaryExpression *unary_syntax = syntax->unary_expression;

        write_syntax(out, unary_syntax->expression, ctx);

        if (unary_syntax->unary_type == BITWISE_NEGATION) {
            emit_insn(out, "not     %eax");
        } else {
            emit_insn(out, "test    $0xFFFFFFFF, %eax");
            emit_insn(out, "setz    %al");
        }
    } else if (syntax->type == IMMEDIATE) {
        fprintf(out, "    mov     $%d, %%eax\n", syntax->value);

    } else if (syntax->type == VARIABLE) {
        fprintf(out, "    mov     %d(%%ebp), %%eax\n",
                environment_get_offset(ctx->env, syntax->variable->var_name));
        
    } else if (syntax->type == BINARY_OPERATOR) {
        BinaryExpression *binary_syntax = syntax->binary_expression;
        int stack_offset = *ctx->stack_offset;
        *ctx->stack_offset -= WORD_SIZE;

        emit_insn(out, "sub     $4, %esp");
        write_syntax(out, binary_syntax->left, ctx);
        fprintf(out, "    mov     %%eax, %d(%%ebp)\n", stack_offset);

        write_syntax(out, binary_syntax->right, ctx);

        if (binary_syntax->binary_type == MULTIPLICATION) {
            fprintf(out, "    mull     %d(%%ebp)\n", stack_offset);
        } else if (binary_syntax->binary_type == ADDITION) {
            fprintf(out, "    add     %d(%%ebp), %%eax\n", stack_offset);
        } else if (binary_syntax->binary_type == LESS_THAN) {
            // To compare x < y in AT&T syntax, we write CMP y,x.
            // http://stackoverflow.com/q/25493255/509706
            fprintf(out, "    cmp     %%eax, %d(%%ebp)\n", stack_offset);
            // Set the low byte of %eax to 0 or 1 depending on whether
            // it was less than.
            emit_insn(out, "setl    %al");
            // Zero the rest of %eax.
            emit_insn(out, "movzbl    %al, %eax");
        }

    } else if (syntax->type == ASSIGNMENT) {
        write_syntax(out, syntax->assignment->expression, ctx);

        fprintf(out, "    mov     %%eax, %d(%%ebp)\n",
                environment_get_offset(ctx->env, syntax->variable->var_name));
        
    } else if (syntax->type == STATEMENT) {
        Statement *statement = syntax->statement;
        
        if (statement->statement_type == RETURN_STATEMENT) {
            write_syntax(out, syntax->statement->return_expression, ctx);
            emit_header(out, "");
            emit_insn(out, "mov     %eax, %ebx");
            emit_insn(out, "mov     $1, %eax");
            emit_insn(out, "int     $0x80");
        }
    } else if (syntax->type == IF_STATEMENT_SYNTAX) {
        IfStatement *if_statement = syntax->if_statement;
        write_syntax(out, if_statement->condition, ctx);

        char *label = fresh_local_label("if_end");

        emit_insn(out, "test    %eax, %eax");
        fprintf(out, "    jz    %s\n", label);

        write_syntax(out, if_statement->then, ctx);
        emit_label(out, label);

    } else if (syntax->type == WHILE_SYNTAX) {
        WhileStatement *while_statement = syntax->while_statement;

        char *start_label = fresh_local_label("while_start");
        char *end_label = fresh_local_label("while_end");

        emit_label(out, start_label);
        write_syntax(out, while_statement->condition, ctx);
        
        emit_insn(out, "test    %eax, %eax");
        fprintf(out, "    jz    %s\n", end_label);

        write_syntax(out, while_statement->body, ctx);
        fprintf(out, "    jmp    %s\n", start_label);
        emit_label(out, end_label);

    } else if (syntax->type == DEFINE_VAR_SYNTAX) {
        DefineVarStatement *define_var_statement = syntax->define_var_statement;
        int stack_offset = *ctx->stack_offset;
            
        environment_set_offset(ctx->env, define_var_statement->var_name, stack_offset);
        emit_insn(out, "sub     $4, %esp");

        *ctx->stack_offset -= WORD_SIZE;
        write_syntax(out, define_var_statement->init_value, ctx);
        fprintf(out, "    mov     %%eax, %d(%%ebp)\n", stack_offset);

    } else if (syntax->type == BLOCK) {
        List *statements = syntax->block->statements;
        for (int i=0; i<list_length(statements); i++) {
            write_syntax(out, list_get(statements, i), ctx);
        }
    } else if (syntax->type == FUNCTION) {
        write_syntax(out, syntax->function->root_block, ctx);
    } else {
        warnx("Unknown syntax %s", syntax_type_name(syntax));
        assert(false);
    }
    
}

void write_assembly(Syntax *syntax) {
    FILE *out = fopen("out.s", "wb");

    write_header(out);

    int *stack_offset = malloc(sizeof(int));
    *stack_offset = -1 * WORD_SIZE;
    
    Context *ctx = malloc(sizeof(Context));
    ctx->stack_offset = stack_offset;
    ctx->env = environment_new();

    write_syntax(out, syntax, ctx);
    
    fclose(out);
}
