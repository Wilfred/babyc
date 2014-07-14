// TODO: this should be a header.
#ifndef BABYC_SYNTAX
#define BABYC_SYNTAX

typedef enum { IMMEDIATE, UNARY_OPERATOR } SyntaxType;

struct Syntax;
typedef struct Syntax Syntax;

struct Syntax {
    SyntaxType type;
    union {
        // Immediate
        int value;

        // Unary operators
        // TODO: store which operator
        struct Syntax* expression;
    };
};

#endif
