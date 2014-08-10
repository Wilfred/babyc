#include <stdlib.h>
#include <stdio.h>
#include "syntax.h"
#include "list.h"

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

Syntax *return_statement_new(Syntax *expression) {
    Syntax *syntax = malloc(sizeof(Syntax));

    StatementSyntax *statement = malloc(sizeof(StatementSyntax));

    statement->statement_type = RETURN_STATEMENT;
    statement->expression = expression;

    syntax->type = STATEMENT;
    syntax->statement = statement;

    return syntax;
}

Syntax *function_new(List *statements) {
    Syntax *syntax = malloc(sizeof(Syntax));

    FunctionSyntax *function = malloc(sizeof(FunctionSyntax));

    function->statements = statements;

    syntax->type = FUNCTION;
    syntax->function = function;

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
        
    } else if (syntax->type == STATEMENT) {
        syntax_free(syntax->statement->expression);
        free(syntax->statement);
        
    } else if (syntax->type == FUNCTION) {
        list_free(syntax->function->statements);
        free(syntax->function);
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
        // TODO: factor out a function: char *syntax_type_string(Syntax *s);
        printf("UNARY ");
        if (syntax->unary_expression->unary_type == BITWISE_NEGATION) {
            printf("BITWISE_NEGATION");
        } else if (syntax->unary_expression->unary_type == LOGICAL_NEGATION) {
            printf("LOGICAL_NEGATION");
        }
        printf("\n");
        
        print_syntax_indented(syntax->unary_expression->expression, indent + 4);
        
    } else if (syntax->type == BINARY_OPERATOR){
        char *type_name = "";
        if (syntax->binary_expression->binary_type == ADDITION) {
            type_name = "ADDITION";
        } else if (syntax->binary_expression->binary_type == MULTIPLICATION) {
            type_name = "MULTIPLICATION";
        }
        
        printf("BINARY %s LEFT\n", type_name);
        print_syntax_indented(syntax->binary_expression->left, indent + 4);

        for (int i=0; i<indent; i++) {
            printf(" ");
        }
        printf("BINARY %s RIGHT\n", type_name);
        print_syntax_indented(syntax->binary_expression->right, indent + 4);
        
    } else if (syntax->type == STATEMENT){
        printf("STATEMENT\n");
        print_syntax_indented(syntax->statement->expression, indent + 4);

    } else if (syntax->type == FUNCTION) {
        printf("FUNCTION\n");

        List *statements = syntax->function->statements;
        for (int i=0; i<list_length(statements); i++) {
            print_syntax_indented(list_get(statements, i), indent + 4);
        }
    }
}

void print_syntax(Syntax *syntax) {
    print_syntax_indented(syntax, 0);
}

