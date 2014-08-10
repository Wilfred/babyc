#include <stdlib.h>
#include <stdio.h>
#include "syntax.h"

Syntax *immediate_new(int value) {
    Syntax *syntax = malloc(sizeof(Syntax));

    syntax->type = IMMEDIATE;
    syntax->value = value;

    return syntax;
}

Syntax *bitwise_negation_new(Syntax *expression) {
    Syntax *syntax = malloc(sizeof(Syntax));

    UnarySyntax *unary_syntax = malloc(sizeof(UnarySyntax));

    unary_syntax->unary_type = BITWISE_NEGATION;
    unary_syntax->expression = expression;

    syntax->type = UNARY_OPERATOR;
    syntax->unary_expression = unary_syntax;

    return syntax;
}

Syntax *logical_negation_new(Syntax *expression) {
    Syntax *syntax = malloc(sizeof(Syntax));

    UnarySyntax *unary_syntax = malloc(sizeof(UnarySyntax));

    unary_syntax->unary_type = LOGICAL_NEGATION;
    unary_syntax->expression = expression;

    syntax->type = UNARY_OPERATOR;
    syntax->unary_expression = unary_syntax;

    return syntax;
}

Syntax *addition_new(Syntax *left, Syntax *right) {
    Syntax *syntax = malloc(sizeof(Syntax));

    BinarySyntax *binary_syntax = malloc(sizeof(BinarySyntax));

    binary_syntax->binary_type = ADDITION;
    binary_syntax->left = left;
    binary_syntax->right = right;

    syntax->type = BINARY_OPERATOR;
    syntax->binary_expression = binary_syntax;

    return syntax;
}

Syntax *multiplication_new(Syntax *left, Syntax *right) {
    Syntax *syntax = malloc(sizeof(Syntax));

    BinarySyntax *binary_syntax = malloc(sizeof(BinarySyntax));

    binary_syntax->binary_type = MULTIPLICATION;
    binary_syntax->left = left;
    binary_syntax->right = right;

    syntax->type = BINARY_OPERATOR;
    syntax->binary_expression = binary_syntax;

    return syntax;
}

Syntax *return_statement_new(Syntax *first) {
    Syntax *syntax = malloc(sizeof(Syntax));

    StatementSyntax *statement = malloc(sizeof(StatementSyntax));

    statement->statement_type = RETURN_STATEMENT;
    statement->first = first;
    statement->second = NULL;

    syntax->type = STATEMENT;
    syntax->statement = statement;

    return syntax;
}

void syntax_free(Syntax *syntax) {
    if (syntax->type == UNARY_OPERATOR) {
        syntax_free(syntax->unary_expression->expression);
        free(syntax->unary_expression);
    } else if (syntax->type == BINARY_OPERATOR){
        syntax_free(syntax->binary_expression->left);
        syntax_free(syntax->binary_expression->right);
        free(syntax->binary_expression);
    } else if (syntax->type == STATEMENT){
        syntax_free(syntax->statement->first);
        if (syntax->statement->second != NULL) {
            syntax_free(syntax->statement->second);
        }
        free(syntax->statement);
    }
    free(syntax);
}

void print_syntax_indented(Syntax *syntax, int indent) {
    for (int i=0; i<indent; i++) {
        printf(" ");
    }

    if (syntax->type == IMMEDIATE) {
        printf("%d\n", syntax->value);
    } else if (syntax->type == UNARY_OPERATOR) {
        printf("UNARY\n");
        print_syntax_indented(syntax->unary_expression->expression, indent + 4);
    } else if (syntax->type == BINARY_OPERATOR){
        printf("BINARY LEFT\n");
        print_syntax_indented(syntax->binary_expression->left, indent + 4);

        for (int i=0; i<indent; i++) {
            printf(" ");
        }
        printf("BINARY RIGHT\n");
        print_syntax_indented(syntax->binary_expression->right, indent + 4);
    } else if (syntax->type == STATEMENT){
        printf("STATEMENT\n");
        print_syntax_indented(syntax->statement->first, indent + 4);
        if (syntax->statement->second != NULL) {
            print_syntax_indented(syntax->statement->second, indent + 4);
        }
    }
}

void print_syntax(Syntax *syntax) {
    print_syntax_indented(syntax, 0);
}

