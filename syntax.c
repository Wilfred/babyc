
/* ----------------------------------------------------------------
 *
 * BabyC Toy compiler for educational purposes
 *
 * ---------------------------------------------------------------- */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "log_error.h"
#include "syntax.h"
#include "ast_annotate.h"

ObjectType convert_type(char *s) {
    ObjectType type = 0;

    if (!strcmp(s, "int")) {
        type = O_INT32;
    } else if (!strcmp(s, "bool")) {
        type = O_UINT1;
    } else if (!strcmp(s, "long")) {
        type = O_INT32;
    } else if (!strcmp(s, "intptr_t")) {
        type = O_INT32;
    } else if (!strcmp(s, "int8_t")) {
        type = O_INT8;
    } else if (!strcmp(s, "int16_t")) {
        type = O_INT16;
    } else if (!strcmp(s, "int32_t")) {
        type = O_INT32;
    } else if (!strcmp(s, "int64_t")) {
        type = O_INT64;
    } else if (!strcmp(s, "int128_t")) {
        type = O_INT128;
    } else if (!strcmp(s, "uint8_t")) {
        type = O_UINT8;
    } else if (!strcmp(s, "uint16_t")) {
        type = O_UINT16;
    } else if (!strcmp(s, "uint32_t")) {
        type = O_UINT32;
    } else if (!strcmp(s, "uint64_t")) {
        type = O_UINT64;
    } else if (!strcmp(s, "uint128_t")) {
        type = O_UINT128;
    } else {
        log_error("Invalid type %s", s);
    }
    return type;
}

Syntax *object_type_size_syntax(ObjectType type) {
    if (type & O_ADDRESS)
        return immediate_new("4");
    else if (type & O_INT8)
        return immediate_new("1");
    else if (type & O_INT16)
        return immediate_new("2");
    else if (type & O_INT32)
        return immediate_new("4");
    else if (type & O_INT64)
        return immediate_new("8");
    else if (type & O_INT128)
        return immediate_new("16");
    return immediate_new("4");
}

int object_type_size_value(ObjectType type) {
    if (type & O_ADDRESS)
        return 4;
    else if (type & O_INT8)
        return 1;
    else if (type & O_INT16)
        return 2;
    else if (type & O_INT32)
        return 4;
    else if (type & O_INT64)
        return 8;
    else if (type & O_INT128)
        return 16;
    return 4;
}

ObjectType syntax_type_size_type(Syntax *syntax) {
    ObjectType type = O_INT32;

    if (syntax->type == BINARY_OPERATOR) {
        type = syntax->binary_expression->objectType;
    } else if (syntax->type == UNARY_OPERATOR) {
        type = syntax->unary_expression->objectType;
    } else if (syntax->type == VARIABLE) {
        type = syntax->variable->objectType;
    } else if (syntax->type == ASSIGNMENT_STATEMENT) {
        type = syntax->assignment->variable->objectType;
    } else if (syntax->type == FUNCTION_CALL) {
        type = syntax->function_call->objectType;
    }
    return type;
}

int syntax_type_size_value(Syntax *syntax) {
    ObjectType type = syntax_type_size_type(syntax);

    if (type & O_ADDRESS)
        return 4;
    else if (type & O_INT8)
        return 1;
    else if (type & O_INT16)
        return 2;
    else if (type & O_INT32)
        return 4;
    else if (type & O_INT64)
        return 8;
    else if (type & O_INT128)
        return 16;
    return 4;
}

Scope *scope_new(Scope *current) {
    Scope *ret = malloc(sizeof(Scope));

    ret->prev_scope = current;
    ret->var_stack = list_new();
    ret->parser_stack = list_new();
    ret->arguments_stack = list_new();
    return ret;
}

Scope *scope_del(Scope *current) {
    Scope *ret = 0;

    if (current) {
        ret = current->prev_scope;
        list_free(current->var_stack);
        list_free(current->parser_stack);
        list_free(current->arguments_stack);
    }
    return ret;
}

void scope_free(Scope *ctx) {
    free(ctx);
}

Variable *scope_get_var(Scope *ctx, char *name) {
    int i;

    do {
        for (i = 0; i < list_length(ctx->var_stack); i++) {
            Variable *v = list_get(ctx->var_stack, i);

            if (!strcmp(v->name, name)) {
                return v;
            }
        }
        ctx = ctx->prev_scope;
    } while (ctx);

    printf("Symbol not found : %s\n", name);
    return 0;
}

Variable *scope_add_var(Scope *ctx, char *name, ObjectType type,
                        Storage storage) {
    Variable *v = malloc(sizeof(Variable));

    memset(v, 0, sizeof(Variable));

    list_push(ctx->var_stack, v);
    v->name = name;
    v->storage = storage;
    v->objectType = type;
    return v;
}

Syntax *nop_new(void) {
    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = NOP_STATEMENT;
    return syntax;
}

Syntax *label_statement_new(Label *label) {
    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = LABEL_STATEMENT;
    syntax->label = label;
    return syntax;
}

Syntax *goto_statement_new(Label *label) {
    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = GOTO_STATEMENT;
    syntax->label = label;
    return syntax;
}

Syntax *immediate_new(char *value) {
    Immediate *immediate = malloc(sizeof(Immediate));

    ast_integer_set_str(&immediate->value, value, 10);

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = IMMEDIATE;
    syntax->immediate = immediate;

    return syntax;
}

Syntax *variable_new(Variable *v) {
    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = VARIABLE;
    syntax->variable = v;

    return syntax;
}

Syntax *address_new(Syntax *expression, Syntax *offset) {
    AddressExpression *address = malloc(sizeof(AddressExpression));

    address->identifier = expression;
    address->offset = offset;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = ADDRESS;
    syntax->address = address;

    return syntax;
}

Syntax *read_pointer_new(Syntax *address, Syntax *offset) {
    ReadAddressExpression *pointer = malloc(sizeof(ReadAddressExpression));

    pointer->address = address;
    pointer->offset = offset;
    pointer->objectType = O_UNDEF;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = READ_ADDRESS;
    syntax->read_address = pointer;

    return syntax;
}

Syntax *write_pointer_new(Syntax *address, Syntax *offset, Syntax *expression) {
    WriteAddressExpression *pointer = malloc(sizeof(WriteAddressExpression));

    pointer->address = address;
    pointer->offset = offset;
    pointer->expression = expression;
    pointer->objectType = O_UNDEF;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = WRITE_ADDRESS;
    syntax->write_address = pointer;

    return syntax;
}

static Syntax *generic_unary_new(Syntax *expression,
                                 UnaryExpressionType unary_type) {
    UnaryExpression *unary_syntax = malloc(sizeof(UnaryExpression));

    unary_syntax->unary_type = unary_type;
    unary_syntax->expression = expression;
    unary_syntax->objectType = O_UNDEF;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = UNARY_OPERATOR;
    syntax->unary_expression = unary_syntax;

    return syntax;
}

static Syntax *generic_binary_new(Syntax *left, Syntax *right,
                                  BinaryExpressionType binary_type) {
    BinaryExpression *binary_syntax = malloc(sizeof(BinaryExpression));

    binary_syntax->binary_type = binary_type;
    binary_syntax->left = left;
    binary_syntax->right = right;
    binary_syntax->objectType = O_UNDEF;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = BINARY_OPERATOR;
    syntax->binary_expression = binary_syntax;

    return syntax;
}

Syntax *bitwise_negation_new(Syntax *expression) {
    return generic_unary_new(expression, BITWISE_NEGATION);
}

Syntax *arithmetic_negation_new(Syntax *expression) {
    return generic_unary_new(expression, ARITHMETIC_NEGATION);
}

Syntax *logical_negation_new(Syntax *expression) {
    return generic_unary_new(expression, LOGICAL_NEGATION);
}

Syntax *addition_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, ADDITION);
}

Syntax *subtraction_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, SUBTRACTION);
}

Syntax *multiplication_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, MULTIPLICATION);
}

Syntax *division_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, DIVISION);
}

Syntax *modulus_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, MODULUS);
}

Syntax *bitwise_xor_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, XOR);
}

Syntax *bitwise_or_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, OR);
}

Syntax *bitwise_and_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, AND);
}

Syntax *logical_or_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, LOGICAL_OR);
}

Syntax *logical_and_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, LOGICAL_AND);
}

Syntax *rshift_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, RSHIFT);
}

Syntax *lshift_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, LSHIFT);
}

Syntax *equal_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, EQUAL);
}

Syntax *nequal_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, NEQUAL);
}

Syntax *less_than_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, LESS_THAN);
}

Syntax *larger_than_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, LARGER_THAN);
}

Syntax *less_or_equal_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, LESS_THAN_OR_EQUAL);
}

Syntax *larger_or_equal_new(Syntax *left, Syntax *right) {
    return generic_binary_new(left, right, LARGER_THAN_OR_EQUAL);
}

Syntax *function_call_new(char *name, List *args) {
    FunctionCall *function_call = malloc(sizeof(FunctionCall));

    function_call->name = name;
    function_call->arguments = args;
    function_call->objectType = O_UNDEF;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = FUNCTION_CALL;
    syntax->function_call = function_call;

    return syntax;
}

Syntax *function_argument_new(Syntax *expression) {
    FunctionArgument *arg = malloc(sizeof(FunctionArgument));

    arg->expression = expression;
    arg->objectType = O_UNDEF;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = FUNCTION_ARGUMENT;
    syntax->function_argument = arg;

    return syntax;
}

Syntax *function_parameter_new(Variable *v) {
    FunctionParameter *parm = malloc(sizeof(FunctionParameter));

    parm->variable = v;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = FUNCTION_PARAMETER;
    syntax->function_parameter = parm;

    return syntax;
}

Syntax *function_definition_new(char *name, ObjectType type, List *parameters,
                                List *labels, Syntax *block) {
    FunctionDefinition *function = malloc(sizeof(FunctionDefinition));

    function->objectType = type;
    function->name = name;
    function->parameters = parameters;
    function->labels = labels;
    function->block = block;
    function->max_dynamic_offset = 0;
    function->max_automatic_offset = 0;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = FUNCTION_DEFINITION;
    syntax->function_definition = function;

    return syntax;
}

Syntax *assignment_new(Variable *v, Syntax *expression) {
    Assignment *assignment = malloc(sizeof(Assignment));

    assignment->variable = v;
    assignment->expression = expression;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = ASSIGNMENT_STATEMENT;
    syntax->assignment = assignment;

    return syntax;
}

Syntax *assignment_static_new(Variable *v, Syntax *expression) {
    if (!expression) {
        expression = immediate_new("0");
    }
    Syntax *syntax = assignment_new(v, expression);

    syntax->type = ASSIGNMENT_STATIC;
    return syntax;
}

Syntax *return_statement_new(Syntax *expression, ObjectType type) {
    ReturnStatement *return_statement = malloc(sizeof(ReturnStatement));

    return_statement->expression = expression;
    return_statement->objectType = type;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = RETURN_STATEMENT;
    syntax->return_statement = return_statement;

    return syntax;
}

Syntax *block_new(List *statements) {
    Block *block = malloc(sizeof(Block));

    block->statements = statements;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = BLOCK;
    syntax->block = block;

    return syntax;
}

Syntax *cast_new(ObjectType type, Syntax *expression) {
    Syntax *syntax = generic_unary_new(expression, CAST);
    syntax->unary_expression->objectType = type;
    return syntax;
}

Syntax *if_new(Syntax *condition, Syntax *if_then, Syntax *if_else) {
    IfStatement *if_statement = malloc(sizeof(IfStatement));

    if_statement->condition = condition;
    if_statement->if_then = if_then;
    if_statement->if_else = if_else;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = IF_STATEMENT;
    syntax->if_statement = if_statement;

    return syntax;
}

Syntax *while_new(Syntax *condition, Syntax *body) {
    WhileStatement *while_statement = malloc(sizeof(WhileStatement));

    while_statement->condition = condition;
    while_statement->body = body;

    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = WHILE_STATEMENT;
    syntax->while_statement = while_statement;

    return syntax;
}

Syntax *break_statement_new(void) {
    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = BREAK_STATEMENT;
    syntax->label = 0;

    return syntax;
}

Syntax *continue_statement_new(void) {
    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = CONTINUE_STATEMENT;
    syntax->label = 0;

    return syntax;
}

void syntax_list_free(List *syntaxes) {
    if (syntaxes == NULL) {
        return;
    }

    for (int i = 0; i < list_length(syntaxes); i++) {
        syntax_free(list_get(syntaxes, i));
    }

    list_free(syntaxes);
}

bool syntax_is_boolean(Syntax *syntax) {
    if (syntax->type == UNARY_OPERATOR) {
        return syntax_is_boolean(syntax->unary_expression->expression);
    }
    if (syntax->type == BINARY_OPERATOR) {
        BinaryExpressionType binary_type =
            syntax->binary_expression->binary_type;

        return (binary_type == LESS_THAN || binary_type == LARGER_THAN ||
                binary_type == LESS_THAN_OR_EQUAL ||
                binary_type == LARGER_THAN_OR_EQUAL || binary_type == EQUAL ||
                binary_type == NEQUAL || binary_type == LOGICAL_OR ||
                binary_type == LOGICAL_AND);
    }
    return false;
}

void syntax_free(Syntax *syntax) {
    if (syntax->type == IMMEDIATE) {
        /* nothing to clean */
        /* TODO : clean variables from parser scopes (ooops, already deleted) */
    } else if (syntax->type == IMMEDIATE) {
        free(syntax->immediate);
    } else if (syntax->type == ADDRESS) {
    } else if (syntax->type == READ_ADDRESS) {
        syntax_free(syntax->read_address->address);
        syntax_free(syntax->read_address->offset);
        free(syntax->read_address);
    } else if (syntax->type == WRITE_ADDRESS) {
        syntax_free(syntax->write_address->address);
        syntax_free(syntax->write_address->offset);
        syntax_free(syntax->write_address->expression);
        free(syntax->write_address);
    } else if (syntax->type == UNARY_OPERATOR) {
        syntax_free(syntax->unary_expression->expression);
        free(syntax->unary_expression);
    } else if (syntax->type == BINARY_OPERATOR) {
        syntax_free(syntax->binary_expression->left);
        syntax_free(syntax->binary_expression->right);
        free(syntax->binary_expression);
    } else if (syntax->type == FUNCTION_CALL) {
        syntax_list_free(syntax->function_call->arguments);
        free(syntax->function_definition);
    } else if (syntax->type == FUNCTION_DEFINITION) {
        syntax_list_free(syntax->function_call->arguments);
        free(syntax->function_call);
    } else if (syntax->type == FUNCTION_ARGUMENT) {
        syntax_free(syntax->function_argument->expression);
        free(syntax->function_argument);
    } else if (syntax->type == FUNCTION_PARAMETER) {
        free(syntax->function_parameter);
    } else if (syntax->type == BREAK_STATEMENT) {
    } else if (syntax->type == CONTINUE_STATEMENT) {
    } else if (syntax->type == GOTO_STATEMENT) {
    } else if (syntax->type == LABEL_STATEMENT) {
    } else if (syntax->type == IF_STATEMENT) {
        syntax_free(syntax->if_statement->condition);
        syntax_free(syntax->if_statement->if_then);
        syntax_free(syntax->if_statement->if_else);
    } else if (syntax->type == RETURN_STATEMENT) {
        syntax_free(syntax->return_statement->expression);
        free(syntax->return_statement);
    } else if (syntax->type == BLOCK) {
        syntax_list_free(syntax->block->statements);
        free(syntax->block);
    } else if (syntax->type == ASSIGNMENT_STATEMENT) {
        syntax_free(syntax->assignment->expression);
        free(syntax->assignment);

    } else if (syntax->type == ASSIGNMENT_STATIC) {
        syntax_free(syntax->assignment->expression);
        free(syntax->assignment);

    } else if (syntax->type == WHILE_STATEMENT) {
        syntax_free(syntax->while_statement->condition);
        syntax_free(syntax->while_statement->body);

    } else {
        printf("Could not free syntax tree with type: %s",
               syntax_type_name(syntax));
        assert(0);
    }

    free(syntax);
}

const char *object_type_name(ObjectType type) {
    if (type & O_ADDRESS) {
        type &= ~O_ADDRESS;
        if (type == O_UINT128)
            return "uint128 *";
        if (type == O_UINT64)
            return "uint64 *";
        if (type == O_UINT32)
            return "uint32 *";
        if (type == O_UINT16)
            return "uint16 *";
        if (type == O_UINT8)
            return "uint8 *";
        if (type == O_INT128)
            return "int128 *";
        if (type == O_INT64)
            return "int64 *";
        if (type == O_INT32)
            return "int32 *";
        if (type == O_INT16)
            return "int16 *";
        if (type == O_INT8)
            return "int8 *";
        if (type == O_BOOL)
            return "int32 *";
        if (type == O_VOID)
            return "uint8 *";
        return "undefined pointer type";
    }
    if (type == O_UINT128)
        return "uint128";
    if (type == O_UINT64)
        return "uint64";
    if (type == O_UINT32)
        return "uint32";
    if (type == O_UINT16)
        return "uint16";
    if (type == O_UINT8)
        return "uint8";
    if (type == O_INT128)
        return "int128";
    if (type == O_INT64)
        return "int64";
    if (type == O_INT32)
        return "int32";
    if (type == O_INT16)
        return "int16";
    if (type == O_INT8)
        return "int8";
    if (type == O_BOOL)
        return "int32";
    if (type == O_VOID)
        return "uint8";
    return "undefined type";
}

const char *syntax_type_name(Syntax *syntax) {
    if (syntax->type == IMMEDIATE) {
        return "IMMEDIATE";
    } else if (syntax->type == VARIABLE) {
        return "VARIABLE";
    } else if (syntax->type == UNARY_OPERATOR) {
        if (syntax->unary_expression->unary_type == BITWISE_NEGATION) {
            return "UNARY BITWISE NEG";
        } else if (syntax->unary_expression->unary_type ==
                   ARITHMETIC_NEGATION) {
            return "UNARY ARITHMETIC NEG";
        } else if (syntax->unary_expression->unary_type == LOGICAL_NEGATION) {
            return "UNARY LOGICAL NEG";
        } else if (syntax->unary_expression->unary_type == CAST) {
            return "UNARY CAST";
        } else
            return "UNARY OP ???";
    } else if (syntax->type == BINARY_OPERATOR) {
        if (syntax->binary_expression->binary_type == ADDITION) {
            return "ADDITION";
        } else if (syntax->binary_expression->binary_type == SUBTRACTION) {
            return "SUBTRACTION";
        } else if (syntax->binary_expression->binary_type == MULTIPLICATION) {
            return "MULTIPLICATION";
        } else if (syntax->binary_expression->binary_type == DIVISION) {
            return "DIVISION";
        } else if (syntax->binary_expression->binary_type == MODULUS) {
            return "MODULUS";
        } else if (syntax->binary_expression->binary_type == LSHIFT) {
            return "LSHIFT";
        } else if (syntax->binary_expression->binary_type == RSHIFT) {
            return "RSHIFT";
        } else if (syntax->binary_expression->binary_type == OR) {
            return "BITWISE OR";
        } else if (syntax->binary_expression->binary_type == AND) {
            return "BITWISE AND";
        } else if (syntax->binary_expression->binary_type == XOR) {
            return "BITWISE XOR";
        } else if (syntax->binary_expression->binary_type == LOGICAL_OR) {
            return "LOGICAL OR";
        } else if (syntax->binary_expression->binary_type == LOGICAL_AND) {
            return "LOGICAL AND";
        } else if (syntax->binary_expression->binary_type == LESS_THAN) {
            return "LESS THAN";
        } else if (syntax->binary_expression->binary_type ==
                   LESS_THAN_OR_EQUAL) {
            return "LESS THAN OR EQUAL";
        } else if (syntax->binary_expression->binary_type == LARGER_THAN) {
            return "LARGER THAN";
        } else if (syntax->binary_expression->binary_type ==
                   LARGER_THAN_OR_EQUAL) {
            return "LARGER THAN OR EQUAL";
        } else if (syntax->binary_expression->binary_type == EQUAL) {
            return "EQUAL";
        } else if (syntax->binary_expression->binary_type == NEQUAL) {
            return "NOT EQUAL";
        } else
            return "BINARY OP ???";
    } else if (syntax->type == ADDRESS) {
        return "ADDRESS";
    } else if (syntax->type == READ_ADDRESS) {
        return "READ ADDRESS";
    } else if (syntax->type == WRITE_ADDRESS) {
        return "WRITE ADDRESS";
    } else if (syntax->type == BLOCK) {
        return "BLOCK";
    } else if (syntax->type == FUNCTION_CALL) {
        return "FUNCTION CALL";
    } else if (syntax->type == FUNCTION_ARGUMENT) {
        return "FUNCTION ARGUMENT";
    } else if (syntax->type == FUNCTION_DEFINITION) {
        return "FUNCTION DEFINITION";
    } else if (syntax->type == FUNCTION_PARAMETER) {
        return "FUNCTION PARAMETER";
    } else if (syntax->type == NOP_STATEMENT) {
        return "NOP";
    } else if (syntax->type == IF_STATEMENT) {
        return "IF";
    } else if (syntax->type == GOTO_STATEMENT) {
        return "GOTO";
    } else if (syntax->type == BREAK_STATEMENT) {
        return "BREAK";
    } else if (syntax->type == CONTINUE_STATEMENT) {
        return "CONTINUE";
    } else if (syntax->type == LABEL_STATEMENT) {
        return "LABEL";
    } else if (syntax->type == RETURN_STATEMENT) {
        return "RETURN";
    } else if (syntax->type == ASSIGNMENT_STATEMENT) {
        return "ASSIGNMENT";
    } else if (syntax->type == ASSIGNMENT_STATIC) {
        return "STATIC ASSIGNMENT";
    } else if (syntax->type == WHILE_STATEMENT) {
        return "WHILE";
    }

    // Should never be reached.
    return "SYNTAX TYPE ????";
}

void print_syntax_indented(Syntax *syntax, int indent) {
    const char *str = syntax_type_name(syntax);

    for (int i = 0; i < indent; i++) {
        printf(" ");
    }

    if (syntax->type == IMMEDIATE) {
        printf("%s '%llu'\n", str,
               ast_integer_get_unsigned_long_long(&syntax->immediate->value));
    } else if (syntax->type == VARIABLE) {
        printf("%s '%s' %s\n", str, syntax->variable->name,
               object_type_name(syntax->variable->objectType));
    } else if (syntax->type == UNARY_OPERATOR) {
        const char *strtype =
            object_type_name(syntax->unary_expression->objectType);

        printf("%s %s\n", str, strtype);
        print_syntax_indented(syntax->unary_expression->expression, indent + 4);

    } else if (syntax->type == BINARY_OPERATOR) {
        const char *strtype =
            object_type_name(syntax->binary_expression->objectType);

        printf("%s %s LEFT\n", str, strtype);
        print_syntax_indented(syntax->binary_expression->left, indent + 4);

        for (int i = 0; i < indent; i++) {
            printf(" ");
        }

        printf("%s %s RIGHT\n", str, strtype);
        print_syntax_indented(syntax->binary_expression->right, indent + 4);

    } else if (syntax->type == FUNCTION_CALL) {
        printf("%s '%s' %s\n", str, syntax->function_call->name,
               object_type_name(syntax->function_call->objectType));

        List *arguments = syntax->function_call->arguments;
        Syntax *argument;

        for (int i = 0; i < list_length(arguments); i++) {
            argument = list_get(arguments, i);
            print_syntax_indented(argument, indent + 4);
        }
    } else if (syntax->type == FUNCTION_DEFINITION) {
        printf("%s '%s' %s\n", str, syntax->function_definition->name,
               object_type_name(syntax->function_definition->objectType));

        List *parameters = syntax->function_definition->parameters;
        Syntax *parameter;

        for (int i = 0; i < list_length(parameters); i++) {
            parameter = list_get(parameters, i);
            print_syntax_indented(parameter, indent + 4);
        }
        print_syntax_indented(syntax->function_definition->block, indent + 4);
    } else if (syntax->type == FUNCTION_PARAMETER) {
        printf(
            "%s '%s' %s\n", str, syntax->function_parameter->variable->name,
            object_type_name(syntax->function_parameter->variable->objectType));
    } else if (syntax->type == FUNCTION_ARGUMENT) {
        printf("%s %s\n", str,
               object_type_name(syntax->function_argument->objectType));
        print_syntax_indented(syntax->function_argument->expression,
                              indent + 4);

    } else if (syntax->type == GOTO_STATEMENT) {
        printf("%s '%s'\n", str, syntax->label->name);
    } else if (syntax->type == BREAK_STATEMENT) {
        printf("%s\n", str);
    } else if (syntax->type == CONTINUE_STATEMENT) {
        printf("%s\n", str);
    } else if (syntax->type == LABEL_STATEMENT) {
        printf("%s '%s'\n", str, syntax->label->name);
    } else if (syntax->type == IF_STATEMENT) {
        printf("%s\n", str);
        print_syntax_indented(syntax->if_statement->condition, indent + 4);

        for (int i = 0; i < indent; i++) {
            printf(" ");
        }

        printf("%s THEN\n", str);
        print_syntax_indented(syntax->if_statement->if_then, indent + 4);

        for (int i = 0; i < indent; i++) {
            printf(" ");
        }

        printf("%s ELSE\n", str);
        print_syntax_indented(syntax->if_statement->if_else, indent + 4);

    } else if (syntax->type == RETURN_STATEMENT) {
        printf("%s %s\n", str,
               object_type_name(syntax->return_statement->objectType));
        print_syntax_indented(syntax->return_statement->expression, indent + 4);
    } else if (syntax->type == ADDRESS) {
        printf("%s\n", str);
        print_syntax_indented(syntax->address->identifier, indent + 4);
        print_syntax_indented(syntax->address->offset, indent + 4);
    } else if (syntax->type == READ_ADDRESS) {
        printf("%s\n", str);
        print_syntax_indented(syntax->read_address->address, indent + 4);
        print_syntax_indented(syntax->read_address->offset, indent + 4);
    } else if (syntax->type == WRITE_ADDRESS) {
        printf("%s\n", str);
        print_syntax_indented(syntax->write_address->address, indent + 4);
        print_syntax_indented(syntax->write_address->offset, indent + 4);
        print_syntax_indented(syntax->write_address->expression, indent + 4);
    } else if (syntax->type == BLOCK) {
        printf("%s\n", str);

        List *statements = syntax->block->statements;

        for (int i = 0; i < list_length(statements); i++) {
            print_syntax_indented(list_get(statements, i), indent + 4);
        }
    } else if (syntax->type == NOP_STATEMENT) {
        printf("%s\n", str);

    } else if (syntax->type == ASSIGNMENT_STATEMENT) {
        printf("%s '%s' %s\n", str, syntax->assignment->variable->name,
               object_type_name(syntax->assignment->variable->objectType));
        print_syntax_indented(syntax->assignment->expression, indent + 4);

    } else if (syntax->type == ASSIGNMENT_STATIC) {
        printf("%s '%s' %s\n", str, syntax->assignment->variable->name,
               object_type_name(syntax->assignment->variable->objectType));
        print_syntax_indented(syntax->assignment->expression, indent + 4);

    } else if (syntax->type == WHILE_STATEMENT) {
        printf("%s\n", str);
        print_syntax_indented(syntax->while_statement->condition, indent + 4);
        print_syntax_indented(syntax->while_statement->body, indent + 4);

    } else {
        printf("??? UNKNOWN SYNTAX TYPE\n");
    }
}

void print_syntax_item(Syntax *syntax) {
    print_syntax_indented(syntax, 0);
}

void print_syntax_list(List *list) {
    for (int i = 0; i < list_length(list); i++) {
        Syntax *syntax = list_get(list, i);

        print_syntax_indented(syntax, 0);
    }
}
