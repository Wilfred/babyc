// TODO: this should be a header.
#ifndef BABYC_SYNTAX
#define BABYC_SYNTAX

typedef enum { IMMEDIATE } SyntaxType;

typedef struct {
    SyntaxType type;
    union {
        int value;
    };
} Syntax;

#endif
