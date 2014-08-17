#include "list.h"

#ifndef BABYC_SYNTAX_HEADER
#define BABYC_SYNTAX_HEADER

typedef enum { IMMEDIATE, UNARY_OPERATOR, BINARY_OPERATOR, STATEMENT, BLOCK, FUNCTION } SyntaxType;
typedef enum { BITWISE_NEGATION, LOGICAL_NEGATION } UnarySyntaxType;
typedef enum { ADDITION, MULTIPLICATION } BinarySyntaxType;
// We already use 'RETURN' as a token name.
typedef enum { RETURN_STATEMENT } StatementSyntaxType;

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

// TODO: drop the 'Syntax' bit.
typedef struct StatementSyntax {
    StatementSyntaxType statement_type;
    Syntax *expression;
} StatementSyntax;

typedef struct Block {
    List *statements;
} Block;

typedef struct FunctionSyntax {
    Syntax *root_block;
} FunctionSyntax;

struct Syntax {
    SyntaxType type;
    union {
        // Immediate
        int value;

        UnarySyntax *unary_expression;

        BinarySyntax *binary_expression;

        StatementSyntax *statement;
        
        FunctionSyntax *function;

        Block *block;
    };
};

Syntax *immediate_new(int value);

Syntax *bitwise_negation_new(Syntax *expression);

Syntax *logical_negation_new(Syntax *expression);

Syntax *addition_new(Syntax *left, Syntax *right);

Syntax *multiplication_new(Syntax *left, Syntax *right);

Syntax *return_statement_new(Syntax *expression);

Syntax *block_new(List *statements);

Syntax *function_new(Syntax *root_block);

void syntax_free(Syntax *syntax);

void print_syntax(Syntax *syntax);

#endif
