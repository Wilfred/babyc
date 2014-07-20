#include <stdlib.h>

// TODO: this should be a header.
#ifndef BABYC_SYNTAX
#define BABYC_SYNTAX

typedef enum { IMMEDIATE, UNARY_OPERATOR } SyntaxType;
typedef enum { BITWISE_NEGATION, LOGICAL_NEGATION } UnarySyntaxType;

struct Syntax;
typedef struct Syntax Syntax;

typedef struct UnarySyntax {
    UnarySyntaxType unary_type;
    Syntax *expression;
} UnarySyntax;

struct Syntax {
    SyntaxType type;
    union {
        // Immediate
        int value;

        // Unary operators
        UnarySyntax *expression;
    };
};

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
    syntax->expression = unary_syntax;

    return syntax;
}

void syntax_free(Syntax *syntax) {
    if (syntax->type == UNARY_OPERATOR) {
        syntax_free(syntax->expression->expression);
    }
    free(syntax);
}

#endif
