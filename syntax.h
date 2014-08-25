#include "list.h"

#ifndef BABYC_SYNTAX_HEADER
#define BABYC_SYNTAX_HEADER

typedef enum {
    IMMEDIATE, VARIABLE,
    UNARY_OPERATOR, BINARY_OPERATOR,
    BLOCK, STATEMENT, IF_STATEMENT_SYNTAX,
    DEFINE_VAR_SYNTAX, FUNCTION,
    ASSIGNMENT,
} SyntaxType;
typedef enum { BITWISE_NEGATION, LOGICAL_NEGATION } UnaryExpressionType;
typedef enum {
    ADDITION, MULTIPLICATION,
    LESS_THAN,
} BinaryExpressionType;
// We already use 'RETURN' and 'IF' as token names. TODO: append TOKEN.
typedef enum {
    RETURN_STATEMENT, IF_STATEMENT,
    DEFINE_VAR_STATEMENT,
} StatementType;

struct Syntax;
typedef struct Syntax Syntax;

typedef struct Variable {
    char *var_name;
} Variable;

typedef struct UnaryExpression {
    UnaryExpressionType unary_type;
    Syntax *expression;
} UnaryExpression;

typedef struct BinaryExpression {
    BinaryExpressionType binary_type;
    Syntax *left;
    Syntax *right;
} BinaryExpression;

typedef struct Assignment {
    char *var_name;
    Syntax *expression;
} Assignment;

typedef struct IfStatement {
    Syntax *condition;
    Syntax *then;
} IfStatement;

typedef struct DefineVarStatement {
    char *var_name;
    Syntax *init_value;
} DefineVarStatement;

typedef struct Statement {
    // TODO: just rename this to type.
    StatementType statement_type;

    union {
        Syntax *return_expression;

        Syntax *if_statement;

        Syntax *define_var_statement;
    };
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
        // Immediate. TODO: Box this.
        int value;

        Variable *variable;

        UnaryExpression *unary_expression;

        BinaryExpression *binary_expression;

        Assignment *assignment;

        // TODO: Add a boxed ReturnStatement, then Statement does not
        // need to be a syntax type (it only occurs in Block lists).
        Statement *statement;

        IfStatement *if_statement;

        DefineVarStatement *define_var_statement;
        
        Block *block;
        
        Function *function;
    };
};

Syntax *immediate_new(int value);

Syntax *variable_new(char *var_name);

Syntax *bitwise_negation_new(Syntax *expression);

Syntax *logical_negation_new(Syntax *expression);

Syntax *addition_new(Syntax *left, Syntax *right);

Syntax *multiplication_new(Syntax *left, Syntax *right);

Syntax *less_than_new(Syntax *left, Syntax *right);

Syntax *assignment_new(char *var_name, Syntax *expression);

Syntax *return_statement_new(Syntax *expression);

Syntax *block_new(List *statements);

Syntax *if_new(Syntax *condition, Syntax *then);

Syntax *define_var_new(char *var_name, Syntax *init_value);

Syntax *function_new(Syntax *root_block);

void syntax_free(Syntax *syntax);

char *syntax_type_name(Syntax *syntax);

void print_syntax(Syntax *syntax);

#endif
