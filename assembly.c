
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

bool peephole_optimize = false;

static const int WORD_SIZE = 4;
static const int MAX_MNEMONIC_LENGTH = 7;

/* -----------------------------------------------------------
 * Helper functions
 * generate bits and parts of the assembly code
 * ----------------------------------------------------------- */

Context *context_new(void) {
    Context *ret = malloc(sizeof(Context));

    ret->label_count = 0;
    ret->while_label = 0;
    ret->offset = 0;
    ret->max_offset = 0;
    ret->function_end_label = "";
    return ret;
}

void context_free(Context *ctx) {
    free(ctx);
}

void context_restart(Context *ctx, int offset, char *end_label) {
    ctx->offset = offset;
    ctx->max_offset = offset;
    ctx->function_end_label = end_label;
}

char *get_var_name(Variable *v) {
    return v->assembler_name;
}

char *fresh_numbered_label(char *prefix, int suffix) {
    // We assume we never write more than 6 chars of digits, plus a '.' and '_'
    // and 'nul' byte.
    size_t buffer_size = strlen(prefix) + 8;
    char *buffer = malloc(buffer_size + 1);

    snprintf(buffer, buffer_size, ".%s_%d", prefix, suffix);
    return buffer;
}

void emit_header(FILE *out, char *name) {
    fprintf(out, "%s\n", name);
}

void emit_line(FILE *out) {
    fprintf(
        out,
        "# --------------------------------------------------------------\n");
}

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

void emit_global(FILE *out, char *name, int size) {
    fprintf(out, "%s:\n", name);
    if (size == 1)
        emit_instr(out, ".byte", "0");
    if (size == 2)
        emit_instr(out, ".word", "0");
    if (size == 4)
        emit_instr(out, ".long", "0");
    if (size == 8)
        emit_instr(out, ".quad", "0");
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
    char *buffer = fresh_numbered_label(prefix, ctx->label_count++);
    return buffer;
}

void emit_label(FILE *out, char *label) {
    fprintf(out, "%s:\n", label);
}

void emit_function_declaration(FILE *out, char *name) {
    emit_line(out);
    fprintf(out, "    .global %s\n", name);
    fprintf(out, "%s:\n", name);
}

void emit_function_prologue(FILE *out, int stack_reserved) {
    fprintf(out, "    enter $%d,$0\n", 0 - stack_reserved);
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

void write_copyright(FILE *out, char *comment) {
    emit_line(out);
    fprintf(out, "# %s\n", comment);
    emit_line(out);
    fprintf(out, "\n");
    emit_header(out, "    .att_syntax prefix");
    fprintf(out, "\n");
}

void write_header(FILE *out, char *comment) {
    emit_line(out);
    fprintf(out, "# %s\n", comment);
    emit_line(out);
    emit_header(out, "    .text");
    fprintf(out, "\n");
}

void write_footer(FILE *out, char *comment) {
    emit_line(out);
    fprintf(out, "# %s\n", comment);
    emit_line(out);
}

void emit_var_address(FILE *out, Variable *v) {
    char *reg = "eax";

    switch (v->storage) {
    case GLOBAL:
        emit_instr_format(out, "movl", "$%s, %%%s", v->assembler_name, reg);
        break;
    case AUTOMATIC:
        emit_instr_format(out, "leal", "%s, %%%s", v->assembler_name, reg);
        break;
    case PARAMETER:
        emit_instr_format(out, "leal", "%s, %%%s", v->assembler_name, reg);
        break;
    default:
        assert(0);
        break;
    }
}

/* -----------------------------------------------------------
 * Detects empty AST branches
 * ----------------------------------------------------------- */

bool empty_list(List *list);
bool empty_syntax(Syntax *syntax);

bool empty_list(List *list) {
    bool empty = true;
    for (int i = 0; empty && i < list_length(list); i++) {
        empty &= empty_syntax(list_get(list, i));
    }
    return empty;
}

bool empty_syntax(Syntax *syntax) {
    if (syntax->type == BLOCK) {
        return empty_list(syntax->block->statements);
    } else if (syntax->type == NOP_STATEMENT) {
        return true;
    }
    return false;
}

/* -----------------------------------------------------------
 * Write the assembly code
 * ----------------------------------------------------------- */

void write_syntax(FILE *out, Syntax *syntax, Context *ctx);
void write_list(FILE *out, List *list, Context *ctx);

void write_list(FILE *out, List *list, Context *ctx) {
    for (int i = 0; i < list_length(list); i++) {
        write_syntax(out, list_get(list, i), ctx);
    }
}

void write_syntax(FILE *out, Syntax *syntax, Context *ctx) {
    // Note stack_offset is the next unused memory address in the
    // stack, so we can use it directly but must adjust it for the next caller.
    if (syntax->type == UNARY_OPERATOR) {
        UnaryExpression *unary_syntax = syntax->unary_expression;

        write_unary_syntax(out, unary_syntax->unary_type,
                           unary_syntax->expression, ctx);
    } else if (syntax->type == IMMEDIATE) {
        emit_instr_format(out, "movl", "$%d, %%eax", syntax->immediate->value);
    } else if (syntax->type == VARIABLE) {
        emit_instr_format(out, "movl", "%s, %%eax",
                          get_var_name(syntax->variable));
    } else if (syntax->type == READ_ADDRESS) {
        write_syntax(out, syntax->read_address->address, ctx);
        if (syntax->read_address->offset->type == IMMEDIATE) {
            int value = syntax->read_address->offset->immediate->value;

            emit_instr_format(out, "movl", "%d(%%eax), %%eax", 4 * value);
        } else {
            emit_instr(out, "pushl", "%eax");
            write_syntax(out, syntax->read_address->offset, ctx);
            emit_instr(out, "popl", "%ecx");
            emit_instr(out, "leal", "(%ecx,%eax,4), %eax");
            emit_instr(out, "movl", "(%eax), %eax");
        }
    } else if (syntax->type == WRITE_ADDRESS) {
        int value = 0;

        write_syntax(out, syntax->write_address->address, ctx);
        emit_instr(out, "pushl", "%eax");
        if (syntax->write_address->offset->type == IMMEDIATE) {
            value = syntax->write_address->offset->immediate->value;
        } else {
            write_syntax(out, syntax->read_address->offset, ctx);
            emit_instr(out, "popl", "%ecx");
            emit_instr(out, "leal", "(%ecx,%eax,4), %eax");
            emit_instr(out, "pushl", "%eax");
        }
        write_syntax(out, syntax->write_address->expression, ctx);
        emit_instr(out, "popl", "%ecx");
        emit_instr_format(out, "movl", "%%eax, %d(%%ecx)", 4 * value);
    } else if (syntax->type == ADDRESS) {
        emit_var_address(out, syntax->address->identifier->variable);
        if (syntax->address->offset->type == IMMEDIATE) {
            int value = syntax->address->offset->immediate->value;

            if (value) {
                emit_instr_format(out, "movl", "$%d, %%ecx", value);
                emit_instr(out, "leal", "(%eax,%ecx,4), %eax");
            }
        } else {
            emit_instr(out, "pushl", "%eax");
            write_syntax(out, syntax->address->offset, ctx);
            emit_instr(out, "popl", "%ecx");
            emit_instr(out, "leal", "(%ecx,%eax,4), %eax");
        }
    } else if (syntax->type == BINARY_OPERATOR) {
        BinaryExpression *binary_syntax = syntax->binary_expression;

        write_binary_syntax(out, binary_syntax->binary_type,
                            binary_syntax->left, binary_syntax->right, ctx);
    } else if (syntax->type == ASSIGNMENT_STATEMENT) {
        write_syntax(out, syntax->assignment->expression, ctx);
        emit_instr_format(out, "movl", "%%eax, %s",
                          get_var_name(syntax->assignment->variable));
    } else if (syntax->type == ASSIGNMENT_STATIC) {
    } else if (syntax->type == NOP_STATEMENT) {
    } else if (syntax->type == RETURN_STATEMENT) {
        ReturnStatement *return_statement = syntax->return_statement;

        write_syntax(out, return_statement->expression, ctx);
        emit_instr_format(out, "jmp", "%s", ctx->function_end_label);
    } else if (syntax->type == GOTO_STATEMENT) {
        emit_instr_format(out, "jmp", "%s", syntax->label->assembler_name);
    } else if (syntax->type == BREAK_STATEMENT) {
        char *buffer = fresh_numbered_label("while_end", ctx->while_label + 1);
        emit_instr_format(out, "jmp", "%s", buffer);
        free(buffer);
    } else if (syntax->type == CONTINUE_STATEMENT) {
        char *buffer = fresh_numbered_label("while_start", ctx->while_label);
        emit_instr_format(out, "jmp", "%s", buffer);
        free(buffer);
    } else if (syntax->type == LABEL_STATEMENT) {
        fprintf(out, "%s:\n", syntax->label->assembler_name);
    } else if (syntax->type == IF_STATEMENT) {
        IfStatement *if_statement = syntax->if_statement;
        char *end_label = fresh_local_label("if_end", ctx);
        char *else_label = fresh_local_label("if_else", ctx);
        bool else_empty = empty_syntax(if_statement->if_else);
        ProcessorFlags flags =
            write_condition_syntax(out, if_statement->condition, ctx);
        if (flags == FLAG_Z_VALID) {
            /* last evaluated to 0 and Z=1 */
            emit_instr_format(out, "jz", "%s", else_label);
        } else if (flags == FLAG_Z_BOOL) {
            /* bool condition is true if Z = 1 */
            emit_instr_format(out, "jnz", "%s", else_label);
        } else if (flags == FLAG_NZ_BOOL) {
            /* bool condition is true if Z = 0 */
            emit_instr_format(out, "jz", "%s", else_label);
        } else if (flags == FLAG_BOOL_VALID) {
            emit_instr(out, "testl", "%eax, %eax");
            emit_instr_format(out, "jz", "%s", else_label);
        } else {
            emit_instr(out, "testl", "%eax, %eax");
            emit_instr_format(out, "jz", "%s", else_label);
        }
        write_syntax(out, if_statement->if_then, ctx);
        if (!else_empty)
            emit_instr_format(out, "jmp", "%s", end_label);
        emit_label(out, else_label);
        if (!else_empty)
            write_syntax(out, if_statement->if_else, ctx);
        emit_label(out, end_label);
    } else if (syntax->type == WHILE_STATEMENT) {
        WhileStatement *while_statement = syntax->while_statement;

        int save_while_label = ctx->while_label;
        ctx->while_label = ctx->label_count;
        char *start_label = fresh_local_label("while_start", ctx);
        char *end_label = fresh_local_label("while_end", ctx);

        emit_label(out, start_label);
        ProcessorFlags flags =
            write_condition_syntax(out, while_statement->condition, ctx);
        if (flags == FLAG_Z_VALID) {
            /* last evaluated to 0 and Z=1 */
            emit_instr_format(out, "jz", "%s", end_label);
        } else if (flags == FLAG_Z_BOOL) {
            /* bool condition is true if Z = 1 */
            emit_instr_format(out, "jnz", "%s", end_label);
        } else if (flags == FLAG_NZ_BOOL) {
            /* bool condition is true if Z = 0 */
            emit_instr_format(out, "jz", "%s", end_label);
        } else if (flags == FLAG_BOOL_VALID) {
            emit_instr(out, "testl", "%eax, %eax");
            emit_instr_format(out, "jz", "%s", end_label);
        } else {
            emit_instr(out, "testl", "%eax, %eax");
            emit_instr_format(out, "jz", "%s", end_label);
        }
        write_syntax(out, while_statement->body, ctx);
        emit_instr_format(out, "jmp", "%s", start_label);
        emit_label(out, end_label);
        ctx->while_label = save_while_label;
    } else if (syntax->type == BLOCK) {
        write_list(out, syntax->block->statements, ctx);
    } else if (syntax->type == FUNCTION_ARGUMENT) {
        write_syntax(out, syntax->function_argument->expression, ctx);
        emit_instr(out, "pushl", "%eax");
    } else if (syntax->type == FUNCTION_PARAMETER) {
    } else if (syntax->type == FUNCTION_CALL) {
        if (!strcmp(syntax->function_call->name, "_alloca")) {
            if (list_length(syntax->function_call->arguments) != 1) {
                log_error("_alloca requires exactly 1 parameter (the size to "
                          "allocate on the stack)\n");
            }
            Syntax *s = list_get(syntax->function_call->arguments, 0);

            if (s->type != FUNCTION_ARGUMENT) {
                log_error("_alloca requires exactly 1 parameter (the size to "
                          "allocate on the stack)\n");
            }
            write_syntax(out, s->function_argument->expression, ctx);
            emit_instr(out, "subl", "%eax, %esp");
            emit_instr(out, "movl", "%esp, %eax");
        } else {
            // Todo : get the right length for each parameter
            int arg_size =
                WORD_SIZE * list_length(syntax->function_call->arguments);

            ctx->offset -= arg_size;
            write_list(out, syntax->function_call->arguments, ctx);
            emit_instr_format(out, "call", syntax->function_call->name);
            if (arg_size) {
                emit_instr_format(out, "addl", "$%d, %%esp", arg_size);
            }
            ctx->offset += arg_size;
        }
    } else if (syntax->type == FUNCTION_DEFINITION) {
        char *label = fresh_local_label("function_end", ctx);

        context_restart(ctx, syntax->function_definition->max_automatic_offset,
                        label);
        emit_function_declaration(out, syntax->function_definition->name);
        emit_function_prologue(
            out,
            syntax->function_definition->max_automatic_offset -
                syntax->function_definition->max_dynamic_offset);
        write_list(out, syntax->function_definition->parameters, ctx);
        write_syntax(out, syntax->function_definition->block, ctx);
        fprintf(out, "\n");
        emit_label(out, label);
        emit_function_epilogue(out);

    } else {
        printf("Unknown syntax %s", syntax_type_name(syntax));
        exit(1);
    }
}

/* ----------------------------------------------------------------------
 * Compute stack needed for expression evaluation
 * TODO : take into acount the variable sizes
 * TODO : the following code should benefit from an implementation based on the
 * reuse of a template iterator (much smaller, and neat)
 * ---------------------------------------------------------------------- */

int update_dynamic_syntax(Syntax *syntax);
int update_dynamic_list(List *list);

int update_dynamic_syntax(Syntax *syntax) {
    if (syntax->type == BLOCK) {
        return update_dynamic_list(syntax->block->statements);
    } else if (syntax->type == UNARY_OPERATOR) {
        return update_dynamic_syntax(syntax->unary_expression->expression);
    } else if (syntax->type == BINARY_OPERATOR) {
        int r = update_dynamic_syntax(syntax->binary_expression->right);
        int l = update_dynamic_syntax(syntax->binary_expression->left);

        return WORD_SIZE + ((r > l) ? r : l);
    } else if (syntax->type == FUNCTION_ARGUMENT) {
        return update_dynamic_syntax(syntax->function_argument->expression);
    } else if (syntax->type == FUNCTION_PARAMETER) {
        return 0;
    } else if (syntax->type == FUNCTION_CALL) {
        return update_dynamic_list(syntax->function_call->arguments);
    } else if (syntax->type == FUNCTION_DEFINITION) {
        return update_dynamic_syntax(syntax->function_definition->block);
    } else if (syntax->type == WHILE_STATEMENT) {
        int b = update_dynamic_syntax(syntax->while_statement->body);
        int c = update_dynamic_syntax(syntax->while_statement->condition);

        return (c > b) ? c : b;
    } else if (syntax->type == IF_STATEMENT) {
        int c = update_dynamic_syntax(syntax->if_statement->condition);
        int t = update_dynamic_syntax(syntax->if_statement->if_then);
        int u = update_dynamic_syntax(syntax->if_statement->if_else);

        c = (c > u) ? c : u;
        return (c > t) ? c : t;
    } else if (syntax->type == GOTO_STATEMENT) {
        return 0;
    } else if (syntax->type == BREAK_STATEMENT) {
        return 0;
    } else if (syntax->type == CONTINUE_STATEMENT) {
        return 0;
    } else if (syntax->type == LABEL_STATEMENT) {
        return 0;
    } else if (syntax->type == NOP_STATEMENT) {
        return 0;
    } else if (syntax->type == ASSIGNMENT_STATEMENT) {
        return update_dynamic_syntax(syntax->assignment->expression);
    } else if (syntax->type == RETURN_STATEMENT) {
        return update_dynamic_syntax(syntax->return_statement->expression);
    } else if (syntax->type == ASSIGNMENT_STATIC) {
        return update_dynamic_syntax(syntax->assignment->expression);
    } else if (syntax->type == VARIABLE) {
        return 0;
    } else if (syntax->type == ADDRESS) {
        int e = update_dynamic_syntax(syntax->address->identifier);
        int a = update_dynamic_syntax(syntax->address->offset);

        return (e > a) ? e : a;
    } else if (syntax->type == READ_ADDRESS) {
        return update_dynamic_syntax(syntax->read_address->address);
    } else if (syntax->type == WRITE_ADDRESS) {
        int a = update_dynamic_syntax(syntax->write_address->address);
        int o = update_dynamic_syntax(syntax->write_address->offset);
        int e = update_dynamic_syntax(syntax->write_address->expression);
        int r = (a > o) ? a : o;

        return WORD_SIZE + ((e > r) ? e : r);
    } else if (syntax->type == IMMEDIATE) {
        return 0;
    }
    return 0;
}

int update_dynamic_list(List *list) {
    int offset = 0;

    for (int i = 0; i < list_length(list); i++) {
        int r = update_dynamic_syntax(list_get(list, i));

        if (r > offset)
            offset = r;
    }
    return offset;
}

/* ----------------------------------------------------------------------
 * Compute the memory location for all local and global variables
 * Local variables are placed on the stack at different offsets $-xxxx(%ebx)
 * Function Parameters are placed on the stack at different offsets $+xxxx(%ebx)
 * Temp calculations are placed on the stack $-xxxx(%ebx) in the red zone
 * Global data are on the heap
 * ---------------------------------------------------------------------- */
typedef struct UpdateOffset {
    int label_count;
    int global_offset;
    int parameter_offset;
    int automatic_offset;
    int max_automatic_offset;
    bool function_read_seen;
    bool function_write_seen;
    bool function_assert_seen;
    bool function_exit_seen;
    bool function_rdtsc_seen;
} UpdateOffset;

void update_offset_size(Variable *v, UpdateOffset *ctx) {
    int sz = 4;
    int offset = 0;
    char temp[20];

    if (v->assembler_name)
        return;
    /*
       TODO other types
       if (!strcmp(v->type, "int"))
       sz = 4;
       else if (!strcmp(v->type, "char"))
       sz = 1;
       else
       assert(0):
     */

    switch (v->storage) {
    case GLOBAL:
        offset = (ctx->global_offset += sz);
        sprintf(temp, "..%s_%d", v->name, ctx->label_count++);
        break;
    case AUTOMATIC:
        offset = (ctx->automatic_offset -= sz);
        sprintf(temp, "%d(%%ebp)", offset);
        if (offset < ctx->max_automatic_offset)
            ctx->max_automatic_offset = offset;
        break;
    case PARAMETER:
        offset = (ctx->parameter_offset += sz);
        sprintf(temp, "%d(%%ebp)", offset);
        break;
    default:
        assert(0);
        break;
    }
    v->offset = offset;
    v->assembler_name = strdup(temp);
}

void update_label(Label *l, UpdateOffset *ctx) {
    if (l->assembler_name)
        return;
    char temp[20];

    sprintf(temp, ".%s_%d", l->name, ctx->label_count++);
    l->assembler_name = strdup(temp);
}

void update_offset_syntax(Syntax *syntax, UpdateOffset *ctx);
void update_offset_list(List *list, UpdateOffset *ctx);

void update_offset_syntax(Syntax *syntax, UpdateOffset *ctx) {
    if (syntax->type == BLOCK) {
        update_offset_list(syntax->block->statements, ctx);
    } else if (syntax->type == UNARY_OPERATOR) {
        update_offset_syntax(syntax->unary_expression->expression, ctx);
    } else if (syntax->type == BINARY_OPERATOR) {
        update_offset_syntax(syntax->binary_expression->left, ctx);
        update_offset_syntax(syntax->binary_expression->right, ctx);
    } else if (syntax->type == FUNCTION_ARGUMENT) {
        update_offset_syntax(syntax->function_argument->expression, ctx);
    } else if (syntax->type == FUNCTION_PARAMETER) {
        update_offset_size(syntax->function_parameter->variable, ctx);
    } else if (syntax->type == FUNCTION_CALL) {
        if (!strcmp(syntax->function_call->name, "_exit")) {
            ctx->function_exit_seen = true;
        }
        if (!strcmp(syntax->function_call->name, "_rdtsc")) {
            ctx->function_rdtsc_seen = true;
        }
        if (!strcmp(syntax->function_call->name, "_read")) {
            ctx->function_read_seen = true;
        }
        if (!strcmp(syntax->function_call->name, "_write")) {
            ctx->function_write_seen = true;
        }
        if (!strcmp(syntax->function_call->name, "_assert")) {
            ctx->function_assert_seen = true;
        }
        update_offset_list(syntax->function_call->arguments, ctx);
    } else if (syntax->type == FUNCTION_DEFINITION) {
        ctx->parameter_offset = 4;
        ctx->automatic_offset = 0;
        ctx->max_automatic_offset = 0;
        update_offset_list(syntax->function_definition->parameters, ctx);
        update_offset_syntax(syntax->function_definition->block, ctx);
        syntax->function_definition->max_automatic_offset =
            ctx->max_automatic_offset;
        syntax->function_definition->max_dynamic_offset =
            update_dynamic_syntax(syntax);
    } else if (syntax->type == WHILE_STATEMENT) {
        update_offset_syntax(syntax->while_statement->body, ctx);
        update_offset_syntax(syntax->while_statement->condition, ctx);
    } else if (syntax->type == IF_STATEMENT) {
        update_offset_syntax(syntax->if_statement->condition, ctx);
        update_offset_syntax(syntax->if_statement->if_then, ctx);
        update_offset_syntax(syntax->if_statement->if_else, ctx);
    } else if (syntax->type == BREAK_STATEMENT) {
    } else if (syntax->type == CONTINUE_STATEMENT) {
    } else if (syntax->type == GOTO_STATEMENT) {
    } else if (syntax->type == LABEL_STATEMENT) {
        update_label(syntax->label, ctx);
    } else if (syntax->type == NOP_STATEMENT) {
    } else if (syntax->type == RETURN_STATEMENT) {
        return update_offset_syntax(syntax->return_statement->expression, ctx);
    } else if (syntax->type == ASSIGNMENT_STATEMENT) {
        update_offset_size(syntax->assignment->variable, ctx);
        update_offset_syntax(syntax->assignment->expression, ctx);
    } else if (syntax->type == ASSIGNMENT_STATIC) {
        update_offset_size(syntax->assignment->variable, ctx);
        update_offset_syntax(syntax->assignment->expression, ctx);
    } else if (syntax->type == VARIABLE) {
        update_offset_size(syntax->variable, ctx);
    } else if (syntax->type == ADDRESS) {
        update_offset_syntax(syntax->address->offset, ctx);
        update_offset_syntax(syntax->address->identifier, ctx);
    } else if (syntax->type == READ_ADDRESS) {
        update_offset_syntax(syntax->read_address->address, ctx);
    } else if (syntax->type == WRITE_ADDRESS) {
        update_offset_syntax(syntax->write_address->address, ctx);
        update_offset_syntax(syntax->write_address->offset, ctx);
        update_offset_syntax(syntax->write_address->expression, ctx);
    }
}

void update_offset_list(List *list, UpdateOffset *ctx) {
    for (int i = 0; i < list_length(list); i++) {
        update_offset_syntax(list_get(list, i), ctx);
    }
}

/* ----------------------------------------------------------------------
 * Write global data and initialization code
 * ---------------------------------------------------------------------- */
void write_global_syntax(FILE *out, Syntax *syntax);
void write_global_list(FILE *out, List *list);

void write_global_entry(FILE *out, Variable *v) {
    if (v->storage == GLOBAL) {
        int sz = 4;

        emit_header(out, "    .data");
        /* TODO : set size from type */
        emit_global(out, v->assembler_name, sz);
    }
}

void write_global_setup(FILE *out, Variable *v, Syntax *expression) {
    if (v->storage == GLOBAL) {
        Context ctx;

        memset(&ctx, 0, sizeof(ctx));
        write_global_entry(out, v);
        emit_header(out, "    .text");
        write_syntax(out, expression, &ctx);
        emit_instr_format(out, "movl", "%%eax, %s", get_var_name(v));
    }
}

void write_global_syntax(FILE *out, Syntax *syntax) {
    if (syntax->type == BLOCK) {
        write_global_list(out, syntax->block->statements);
    } else if (syntax->type == FUNCTION_DEFINITION) {
        write_global_list(out, syntax->function_definition->parameters);
        write_global_syntax(out, syntax->function_definition->block);
    } else if (syntax->type == WHILE_STATEMENT) {
        write_global_syntax(out, syntax->while_statement->body);
        write_global_syntax(out, syntax->while_statement->condition);
    } else if (syntax->type == IF_STATEMENT) {
        write_global_syntax(out, syntax->if_statement->condition);
        write_global_syntax(out, syntax->if_statement->if_then);
        write_global_syntax(out, syntax->if_statement->if_else);
    } else if (syntax->type == ASSIGNMENT_STATIC) {
        write_global_setup(out, syntax->assignment->variable,
                           syntax->assignment->expression);
        write_global_syntax(out, syntax->assignment->expression);
    }
}

void write_global_list(FILE *out, List *list) {
    for (int i = 0; i < list_length(list); i++) {
        write_global_syntax(out, list_get(list, i));
    }
}

void write_global(FILE *out, Syntax *syntax) {
    write_global_syntax(out, syntax);
}

/* ----------------------------------------------------------------------
 * Write stdlib as a static library
 *
 * int read(0, addr, 4);    // read a binary number from stdin
 * int write(1, addr, 4);   // write a binary number to stdout
 * void assert(0);          // terminate
 * void assert(!0);         // continue
 *
 * ---------------------------------------------------------------------- */
void write_stdlib(FILE *out, UpdateOffset *ctx) {
    if (ctx->function_read_seen || ctx->function_rdtsc_seen ||
        ctx->function_write_seen || ctx->function_assert_seen ||
        ctx->function_exit_seen) {
        write_header(out, "Stdlib");
    }
    if (ctx->function_exit_seen) {
        fprintf(out, "    .global %s\n", "_exit");
        fprintf(out, "%s:\n", "_exit");
        emit_instr(out, "enter", "$0, $0");
        emit_instr(out, "movl", "8(%ebp), %ebx");
        emit_instr(out, "jmp", ".terminate");
    }
    if (ctx->function_rdtsc_seen) {
        fprintf(out, "    .global %s\n", "_rdtsc");
        fprintf(out, "%s:\n", "_rdtsc");
        emit_instr(out, "rdtsc", "");
        emit_instr(out, "ret", "");
    }
    if (ctx->function_assert_seen) {
        fprintf(out, "    .global %s\n", "_assert");
        fprintf(out, "%s:\n", "_assert");
        emit_instr(out, "enter", "$0, $0");
        emit_instr(out, "movl", "8(%ebp), %ebx");
        emit_instr(out, "cmpl", "$0, %ebx");
        emit_instr(out, "setz", "%bl");
        emit_instr(out, "jz", ".terminate");
        emit_instr(out, "leave", "");
        emit_instr(out, "ret", "");
    }
    if (ctx->function_read_seen) {
        fprintf(out, "    .global %s\n", "_read");
        fprintf(out, "%s:\n", "_read");
        emit_instr(out, "enter", "$0, $0");
        emit_instr(out, "movl", "$3, %eax");
        if (ctx->function_write_seen) {
            emit_instr(out, "jmp", ".write");
        } else {
            emit_instr(out, "movl", "8(%ebp), %ebx");
            emit_instr(out, "movl", "12(%ebp), %ecx");
            emit_instr(out, "movl", "16(%ebp), %edx");
            emit_instr(out, "int", "$0x80");
            emit_instr(out, "leave", "");
            emit_instr(out, "ret", "");
        }
    }
    if (ctx->function_write_seen) {
        fprintf(out, "    .global %s\n", "_write");
        fprintf(out, "%s:\n", "_write");
        emit_instr(out, "enter", "$0, $0");
        emit_instr(out, "movl", "$4, %eax");
        if (ctx->function_read_seen) {
            fprintf(out, "%s:\n", ".write");
        }
        emit_instr(out, "movl", "8(%ebp), %ebx");
        emit_instr(out, "movl", "12(%ebp), %ecx");
        emit_instr(out, "movl", "16(%ebp), %edx");
        emit_instr(out, "int", "$0x80");
        emit_instr(out, "leave", "");
        emit_instr(out, "ret", "");
    }
}

/* ----------------------------------------------------------------------
 * Generate assembly file
 * ---------------------------------------------------------------------- */
void write_assembly(char *file_name, Syntax *syntax) {
    UpdateOffset u;

    /* ------------------------------------------------------
     * Annotate AST (assign variables to memory locations)
     * ------------------------------------------------------ */
    memset(&u, 0, sizeof(u));
    u.global_offset = -4;
    u.parameter_offset = 4;
    update_offset_syntax(syntax, &u);

    FILE *out = fopen(file_name, "wt");

    write_copyright(out, "Generated by Babyc : Simple C Compiler (for "
                         "educational purposes)\n# "
                         "https://github.com/Wilfred/babyc");
    fprintf(out, "\n");

    /* ------------------------------------------------------
     * declare and initialize global data
     * ------------------------------------------------------ */
    write_header(out, "Program entry point and global data initializations");
    fprintf(out, "    .global %s\n", "_start");
    fprintf(out, "%s:\n", "_start");
    emit_instr(out, "movl", "%esp, %ebp");
    write_global(out, syntax);

    /* ------------------------------------------------------
     * enter / exit code
     * ------------------------------------------------------ */
    emit_instr(out, "call", "main");
    emit_instr(out, "movl", "%eax, %ebx");
    fprintf(out, "%s:\n", ".terminate");
    emit_instr(out, "movl", "$1, %eax");
    emit_instr(out, "int", "$0x80");

    /* ------------------------------------------------------
     * static stdlib library
     * ------------------------------------------------------ */
    write_stdlib(out, &u);

    /* ------------------------------------------------------
     * convert AST to asm
     * ------------------------------------------------------ */
    Context *ctx = context_new();

    fprintf(out, "\n");
    write_header(out, "Program code (start)");
    write_syntax(out, syntax, ctx);
    write_footer(out, "Program code (end)");
    fprintf(out, "\n");

    /* ------------------------------------------------------
     * Done
     * ------------------------------------------------------ */
    context_free(ctx);
    fclose(out);
}
