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

char *syntax_type_name(Syntax *syntax) {
    if (syntax->type == UNARY_OPERATOR) {
        if (syntax->unary_expression->unary_type == BITWISE_NEGATION) {
            return "UNARY BITWISE_NEGATION";
        } else if (syntax->unary_expression->unary_type == LOGICAL_NEGATION) {
            return "UNARY BITWISE_NEGATION";
        }
    } else if (syntax->type == BINARY_OPERATOR) {
        if (syntax->binary_expression->binary_type == ADDITION) {
            return "ADDITION";
        } else if (syntax->binary_expression->binary_type == MULTIPLICATION) {
            return "MULTIPLICATION";
        }
    } else if (syntax->type == STATEMENT){
        return "STATEMENT";
    } else if (syntax->type == FUNCTION) {
        return "FUNCTION";
    }

    // Immediates or anything else we don't bother showing when
    // printing the AST.
    return "???";
}

void print_syntax_indented(Syntax *syntax, int indent) {
    for (int i=0; i<indent; i++) {
        printf(" ");
    }

    char *syntax_type_string = syntax_type_name(syntax);

    if (syntax->type == IMMEDIATE) {
        printf("%d\n", syntax->value);
    } else if (syntax->type == UNARY_OPERATOR) {
        printf("%s\n", syntax_type_string);
        print_syntax_indented(syntax->unary_expression->expression, indent + 4);
        
    } else if (syntax->type == BINARY_OPERATOR){
        printf("%s LEFT\n", syntax_type_string);
        print_syntax_indented(syntax->binary_expression->left, indent + 4);

        for (int i=0; i<indent; i++) {
            printf(" ");
        }

        printf("%s RIGHT\n", syntax_type_string);
        print_syntax_indented(syntax->binary_expression->right, indent + 4);
        
    } else if (syntax->type == STATEMENT){
        printf("%s\n", syntax_type_string);
        print_syntax_indented(syntax->statement->expression, indent + 4);

    } else if (syntax->type == FUNCTION) {
        printf("%s\n", syntax_type_string);

        List *statements = syntax->function->statements;
        for (int i=0; i<list_length(statements); i++) {
            print_syntax_indented(list_get(statements, i), indent + 4);
        }
    }
}

void print_syntax(Syntax *syntax) {
    print_syntax_indented(syntax, 0);
}
