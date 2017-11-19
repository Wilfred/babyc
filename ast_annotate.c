
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

bool ast_optimize = false;

ObjectType ast_annotate_syntax(Syntax *syntax, ObjectType dest);
ObjectType ast_annotate_list(List *list, ObjectType dest);
ObjectType ast_convert_value_to_type(char *hexvalue)
{
    char *pt = hexvalue + strlen(hexvalue);
    ObjectType type = O_UINT8;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    type = O_UINT16;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    type = O_UINT32;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    type = O_UINT64;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    if (--pt == hexvalue) return type;
    type = O_UINT128;
    return type;
}

ObjectType ast_annotate_syntax(Syntax *syntax, ObjectType destType) {
    ObjectType type;
    if (syntax->type == BLOCK) {
        type = ast_annotate_list(syntax->block->statements, O_UNDEF);
    } else if (syntax->type == UNARY_OPERATOR) {
        type = ast_annotate_syntax(syntax->unary_expression->expression, destType);
        if (syntax->unary_expression->objectType == O_UNDEF) {
            syntax->unary_expression->objectType = type;
        }
    } else if (syntax->type == BINARY_OPERATOR) {
        ObjectType r = ast_annotate_syntax(syntax->binary_expression->right, destType);
        ObjectType l = ast_annotate_syntax(syntax->binary_expression->left, destType);
        type = (r > l) ? r : l;
        if (syntax->binary_expression->objectType == O_UNDEF) {
            syntax->binary_expression->objectType = type;
        }
    } else if (syntax->type == FUNCTION_ARGUMENT) {
        // todo : get type from function parameters
        type = ast_annotate_syntax(syntax->function_argument->expression, O_UNDEF);
    } else if (syntax->type == FUNCTION_PARAMETER) {
        type = O_UNDEF;
    } else if (syntax->type == FUNCTION_CALL) {
        ast_annotate_list(syntax->function_call->arguments, O_UNDEF);
        // todo get type from functiion defintion
        type = O_UNDEF;
    } else if (syntax->type == FUNCTION_DEFINITION) {
        ast_annotate_syntax(syntax->function_definition->block, O_UNDEF);
        type = O_UNDEF;
    } else if (syntax->type == WHILE_STATEMENT) {
        ast_annotate_syntax(syntax->while_statement->body, O_UNDEF);
        ast_annotate_syntax(syntax->while_statement->condition, O_BOOL);
        type = O_UNDEF;
    } else if (syntax->type == IF_STATEMENT) {
        ast_annotate_syntax(syntax->if_statement->condition, O_BOOL);
        ObjectType r = ast_annotate_syntax(syntax->if_statement->if_then, O_UNDEF);
        ObjectType l = ast_annotate_syntax(syntax->if_statement->if_else, O_UNDEF);
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
        type = ast_annotate_syntax(syntax->assignment->expression, type);
    } else if (syntax->type == RETURN_STATEMENT) {
        // todo get type from function defintion
        type = ast_annotate_syntax(syntax->return_statement->expression, O_UNDEF);
    } else if (syntax->type == ASSIGNMENT_STATIC) {
        type = syntax->assignment->variable->objectType;
        type = ast_annotate_syntax(syntax->assignment->expression, type);
    } else if (syntax->type == VARIABLE) {
        type = syntax->variable->objectType;
    } else if (syntax->type == ADDRESS) {
        ast_annotate_syntax(syntax->address->offset, O_INT32);
        type = ast_annotate_syntax(syntax->address->identifier, destType);
    } else if (syntax->type == READ_ADDRESS) {
        type = ast_annotate_syntax(syntax->read_address->address,destType);
    } else if (syntax->type == WRITE_ADDRESS) {
        type = ast_annotate_syntax(syntax->write_address->address, O_UNDEF);
        ast_annotate_syntax(syntax->write_address->offset, O_INT32);
        type = ast_annotate_syntax(syntax->write_address->expression, type);
    } else if (syntax->type == IMMEDIATE) {
        // TODO : handle any constant size
        int value = syntax->immediate->value;
        char temp[30];
        sprintf(temp, "%x", value);
        type = ast_convert_value_to_type(temp);
        type = (type > destType) ? type : destType;
    }
    else
    {
        type = O_UNDEF;
    }
    return type;
}

ObjectType ast_annotate_list(List *list, ObjectType destType) {

    for (int i = 0; i < list_length(list); i++) {
        ast_annotate_syntax(list_get(list, i), destType);
    }
    return O_UNDEF;
}

