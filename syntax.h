#include "list.h"

#ifndef BABYC_SYNTAX_HEADER
#define BABYC_SYNTAX_HEADER

typedef enum { IMMEDIATE, UNARY_OPERATOR, BINARY_OPERATOR, STATEMENT, BLOCK, FUNCTION } SyntaxType;
typedef enum { BITWISE_NEGATION, LOGICAL_NEGATION } UnaryExpressionType;
typedef enum { ADDITION, MULTIPLICATION } BinaryExpressionType;
// We already use 'RETURN' as a token name.
typedef enum { RETURN_STATEMENT } StatementType;

struct Syntax;
typedef struct Syntax Syntax;

typedef struct UnaryExpression {
    UnaryExpressionType unary_type;
    Syntax *expression;
} UnaryExpression;

typedef struct BinaryExpression {
    BinaryExpressionType binary_type;
    Syntax *left;
    Syntax *right;
} BinaryExpression;

typedef struct Statement {
    StatementType statement_type;
    Syntax *expression;
} Statement;

typedef struct Block {
    List *statements;
} Block;

typedef struct Function {
    Syntax *root_block;
} Function;

struct Syntax {
    SyntaxType type;
    union {
        // Immediate
        int value;

        UnaryExpression *unary_expression;

        BinaryExpression *binary_expression;

        Statement *statement;
        
        Function *function;

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

char *syntax_type_name(Syntax *syntax);

void print_syntax(Syntax *syntax);

#endif
