
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

#include "syntax.h"
#include "log_error.h"
#include "list.h"

bool ast_optimize = false;

/* ----------------------------------------------------------------
 * Get the right type on intermediate AST values
 * ---------------------------------------------------------------- */

ObjectType ast_annotate_expressions_syntax(Syntax *syntax, ObjectType dest);
ObjectType ast_annotate_expressions_list(List *list, ObjectType dest);
ObjectType ast_convert_value_to_type(char *hexvalue) {
    char *pt = hexvalue + strlen(hexvalue);
    ObjectType type = O_UINT8;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    type = O_UINT16;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    type = O_UINT32;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    type = O_UINT64;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    type = O_UINT128;
    return type;
}

ObjectType ast_annotate_expressions_syntax(Syntax *syntax,
                                           ObjectType destType) {
    ObjectType type;
    if (syntax->type == BLOCK) {
        type =
            ast_annotate_expressions_list(syntax->block->statements, O_UNDEF);
    } else if (syntax->type == UNARY_OPERATOR) {
        type = ast_annotate_expressions_syntax(
            syntax->unary_expression->expression, destType);
        if (syntax->unary_expression->objectType == O_UNDEF) {
            syntax->unary_expression->objectType = type;
        }
    } else if (syntax->type == BINARY_OPERATOR) {
        ObjectType r = ast_annotate_expressions_syntax(
            syntax->binary_expression->right, destType);
        ObjectType l = ast_annotate_expressions_syntax(
            syntax->binary_expression->left, destType);
        type = (r > l) ? r : l;
        if (syntax->binary_expression->objectType == O_UNDEF) {
            syntax->binary_expression->objectType = type;
        }
    } else if (syntax->type == FUNCTION_ARGUMENT) {
        type = syntax->function_argument->objectType;
        ast_annotate_expressions_syntax(syntax->function_argument->expression,
                                        type);
    } else if (syntax->type == FUNCTION_PARAMETER) {
        type = syntax->function_parameter->variable->objectType;
    } else if (syntax->type == FUNCTION_CALL) {
        ast_annotate_expressions_list(syntax->function_call->arguments,
                                      O_UNDEF);
        type = syntax->function_call->objectType;
    } else if (syntax->type == FUNCTION_DEFINITION) {
        ast_annotate_expressions_syntax(syntax->function_definition->block,
                                        O_UNDEF);
        type = syntax->function_definition->objectType;
    } else if (syntax->type == WHILE_STATEMENT) {
        ast_annotate_expressions_syntax(syntax->while_statement->body, O_UNDEF);
        ast_annotate_expressions_syntax(syntax->while_statement->condition,
                                        O_BOOL);
        type = O_UNDEF;
    } else if (syntax->type == IF_STATEMENT) {
        ast_annotate_expressions_syntax(syntax->if_statement->condition,
                                        O_BOOL);
        ObjectType r = ast_annotate_expressions_syntax(
            syntax->if_statement->if_then, O_UNDEF);
        ObjectType l = ast_annotate_expressions_syntax(
            syntax->if_statement->if_else, O_UNDEF);
        type = (r > l) ? r : l;
    } else if (syntax->type == GOTO_STATEMENT) {
        type = O_UNDEF;
    } else if (syntax->type == BREAK_STATEMENT) {
        type = O_UNDEF;
    } else if (syntax->type == CONTINUE_STATEMENT) {
        type = O_UNDEF;
    } else if (syntax->type == LABEL_STATEMENT) {
        type = O_UNDEF;
    } else if (syntax->type == NOP_STATEMENT) {
        type = O_UNDEF;
    } else if (syntax->type == ASSIGNMENT_STATEMENT) {
        type = syntax->assignment->variable->objectType;
        ast_annotate_expressions_syntax(syntax->assignment->expression, type);
    } else if (syntax->type == RETURN_STATEMENT) {
        type = syntax->return_statement->objectType;
        ast_annotate_expressions_syntax(syntax->return_statement->expression,
                                        type);
    } else if (syntax->type == ASSIGNMENT_STATIC) {
        type = syntax->assignment->variable->objectType;
        ast_annotate_expressions_syntax(syntax->assignment->expression, type);
    } else if (syntax->type == VARIABLE) {
        type = syntax->variable->objectType;
    } else if (syntax->type == ADDRESS) {
        ast_annotate_expressions_syntax(syntax->address->offset, O_INT32);
        type = ast_annotate_expressions_syntax(syntax->address->identifier,
                                               destType);
    } else if (syntax->type == READ_ADDRESS) {
        type = ast_annotate_expressions_syntax(syntax->read_address->address,
                                               destType);
    } else if (syntax->type == WRITE_ADDRESS) {
        type = ast_annotate_expressions_syntax(syntax->write_address->address,
                                               O_UNDEF);
        ast_annotate_expressions_syntax(syntax->write_address->offset, O_INT32);
        type = ast_annotate_expressions_syntax(
            syntax->write_address->expression, type);
    } else if (syntax->type == IMMEDIATE) {
        // TODO : handle any constant size
        int value = syntax->immediate->value;
        char temp[30];
        sprintf(temp, "%x", value);
        type = ast_convert_value_to_type(temp);
        type = (type > destType) ? type : destType;
    } else {
        type = O_UNDEF;
    }
    return type;
}

ObjectType ast_annotate_expressions_list(List *list, ObjectType destType) {

    for (int i = 0; i < list_length(list); i++) {
        ast_annotate_expressions_syntax(list_get(list, i), destType);
    }
    return O_UNDEF;
}

/* ----------------------------------------------------------------
 * Resolve function calls and function parameters
 * ---------------------------------------------------------------- */
void ast_annotate_function_calls_syntax(Syntax *syntax, Syntax *root);
void ast_annotate_function_calls_list(List *list, Syntax *root);

void ast_link_function_definition(FunctionCall *function_call, Syntax *root) {
    if (root->type != BLOCK) {
        log_error("Unvalid AST root");
    }
    char *name = function_call->name;
    int alen = list_length(function_call->arguments);
    List *top = root->block->statements;
    int jlen = list_length(top);
    for (int j = 0; j < jlen; j++) {
        Syntax *s = list_get(top, j);
        if (s->type == FUNCTION_DEFINITION) {
            FunctionDefinition *function_definition = s->function_definition;
            if (!strcmp(name, function_definition->name)) {
                int plen = list_length(function_definition->parameters);
                if (plen != alen) {
                    log_error("Parameter mismatch on function call '%s'", name);
                }
                function_call->objectType = function_definition->objectType;
                for (int i = 0; i < alen; i++) {
                    Syntax *sp = list_get(function_definition->parameters, i);
                    Syntax *sa = list_get(function_call->arguments, i);
                    FunctionParameter *p = sp->function_parameter;
                    FunctionArgument *a = sa->function_argument;
                    a->objectType = p->variable->objectType;
                }
                return;
            }
        }
    }
}

void ast_annotate_function_calls_syntax(Syntax *syntax, Syntax *root) {
    if (syntax->type == BLOCK) {
        ast_annotate_function_calls_list(syntax->block->statements, root);
    } else if (syntax->type == UNARY_OPERATOR) {
        ast_annotate_function_calls_syntax(syntax->unary_expression->expression,
                                           root);
    } else if (syntax->type == BINARY_OPERATOR) {
        ast_annotate_function_calls_syntax(syntax->binary_expression->right,
                                           root);
        ast_annotate_function_calls_syntax(syntax->binary_expression->left,
                                           root);
    } else if (syntax->type == FUNCTION_ARGUMENT) {
        // todo : get type from function parameters
        ast_annotate_function_calls_syntax(
            syntax->function_argument->expression, root);
    } else if (syntax->type == FUNCTION_PARAMETER) {
    } else if (syntax->type == FUNCTION_CALL) {
        ast_link_function_definition(syntax->function_call, root);
        ast_annotate_function_calls_list(syntax->function_call->arguments,
                                         root);
    } else if (syntax->type == FUNCTION_DEFINITION) {
        ast_annotate_function_calls_syntax(syntax->function_definition->block,
                                           root);
    } else if (syntax->type == WHILE_STATEMENT) {
        ast_annotate_function_calls_syntax(syntax->while_statement->body, root);
        ast_annotate_function_calls_syntax(syntax->while_statement->condition,
                                           root);
    } else if (syntax->type == IF_STATEMENT) {
        ast_annotate_function_calls_syntax(syntax->if_statement->condition,
                                           root);
        ast_annotate_function_calls_syntax(syntax->if_statement->if_then, root);
        ast_annotate_function_calls_syntax(syntax->if_statement->if_else, root);
    } else if (syntax->type == GOTO_STATEMENT) {
    } else if (syntax->type == BREAK_STATEMENT) {
    } else if (syntax->type == CONTINUE_STATEMENT) {
    } else if (syntax->type == LABEL_STATEMENT) {
    } else if (syntax->type == NOP_STATEMENT) {
    } else if (syntax->type == ASSIGNMENT_STATEMENT) {
        ast_annotate_function_calls_syntax(syntax->assignment->expression,
                                           root);
    } else if (syntax->type == RETURN_STATEMENT) {
        ast_annotate_function_calls_syntax(syntax->return_statement->expression,
                                           root);
    } else if (syntax->type == ASSIGNMENT_STATIC) {
        ast_annotate_function_calls_syntax(syntax->assignment->expression,
                                           root);
    } else if (syntax->type == VARIABLE) {
    } else if (syntax->type == ADDRESS) {
        ast_annotate_function_calls_syntax(syntax->address->offset, root);
        ast_annotate_function_calls_syntax(syntax->address->identifier, root);
    } else if (syntax->type == READ_ADDRESS) {
        ast_annotate_function_calls_syntax(syntax->read_address->address, root);
    } else if (syntax->type == WRITE_ADDRESS) {
        ast_annotate_function_calls_syntax(syntax->write_address->address,
                                           root);
        ast_annotate_function_calls_syntax(syntax->write_address->offset, root);
        ast_annotate_function_calls_syntax(syntax->write_address->expression,
                                           root);
    } else if (syntax->type == IMMEDIATE) {
    }
}

void ast_annotate_function_calls_list(List *list, Syntax *root) {
    for (int i = 0; i < list_length(list); i++) {
        ast_annotate_function_calls_syntax(list_get(list, i), root);
    }
}

void ast_annotate_syntax_tree(Syntax *syntax) {
    ast_annotate_function_calls_syntax(syntax, syntax);

    ast_annotate_expressions_syntax(syntax, O_UNDEF);
}
