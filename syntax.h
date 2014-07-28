#include <stdlib.h>

#ifndef BABYC_SYNTAX_HEADER
#define BABYC_SYNTAX_HEADER

typedef enum { IMMEDIATE, UNARY_OPERATOR, BINARY_OPERATOR } SyntaxType;
typedef enum { BITWISE_NEGATION, LOGICAL_NEGATION } UnarySyntaxType;
typedef enum { ADDITION } BinarySyntaxType;

struct Syntax;
typedef struct Syntax Syntax;

typedef struct UnarySyntax {
    UnarySyntaxType unary_type;
    Syntax *expression;
} UnarySyntax;

typedef struct BinarySyntax {
    BinarySyntaxType binary_type;
    Syntax *left;
    Syntax *right;
} BinarySyntax;

struct Syntax {
    SyntaxType type;
    union {
        // Immediate
        int value;

        UnarySyntax *unary_expression;

        BinarySyntax *binary_expression;
    };
};

Syntax *immediate_new(int value);

Syntax *bitwise_negation_new(Syntax *expression);

Syntax *logical_negation_new(Syntax *expression);

Syntax *addition_new(Syntax *left, Syntax *right);

void syntax_free(Syntax *syntax);

#endif
