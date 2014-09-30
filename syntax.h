#include "list.h"

#ifndef BABYC_SYNTAX_HEADER
#define BABYC_SYNTAX_HEADER

typedef enum {
    IMMEDIATE, VARIABLE,
    UNARY_OPERATOR, BINARY_OPERATOR,
    // We already use 'RETURN' and 'IF' as token names.
    BLOCK, IF_STATEMENT,
    RETURN_STATEMENT,
    DEFINE_VAR, FUNCTION,
    FUNCTION_CALL, FUNCTION_ARGUMENTS,
    ASSIGNMENT, WHILE_SYNTAX,
    TOP_LEVEL
} SyntaxType;
typedef enum { BITWISE_NEGATION, LOGICAL_NEGATION } UnaryExpressionType;
typedef enum {
    ADDITION, MULTIPLICATION,
    LESS_THAN,
} BinaryExpressionType;

struct Syntax;
typedef struct Syntax Syntax;

typedef struct Immediate {
    int value;
} Immediate;

typedef struct Variable {
    // TODO: once we have other types, we will need to store type here.
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

typedef struct FunctionArguments {
    List *arguments;
} FunctionArguments;

typedef struct FunctionCall {
    char *function_name;
    Syntax *function_arguments;
} FunctionCall;

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

typedef struct WhileStatement {
    Syntax *condition;
    Syntax *body;
} WhileStatement;

typedef struct ReturnStatement {
    Syntax *expression;
} ReturnStatement;

typedef struct Block {
    List *statements;
} Block;

typedef struct Function {
    char *name;
    List *parameters;
    Syntax *root_block;
} Function;

typedef struct Parameter {
    // TODO: once we have other types, we will need to store type here.
    char *name;
} Parameter;

typedef struct TopLevel {
    List *declarations;
} TopLevel;

struct Syntax {
    SyntaxType type;
    union {
        Immediate *immediate;

        Variable *variable;

        UnaryExpression *unary_expression;

        BinaryExpression *binary_expression;

        Assignment *assignment;

        ReturnStatement *return_statement;

        FunctionArguments *function_arguments;

        FunctionCall *function_call;

        IfStatement *if_statement;

        DefineVarStatement *define_var_statement;

        WhileStatement *while_statement;
        
        Block *block;
        
        Function *function;

        TopLevel *top_level;
    };
};

Syntax *immediate_new(int value);

Syntax *variable_new(char *var_name);

Syntax *bitwise_negation_new(Syntax *expression);

Syntax *logical_negation_new(Syntax *expression);

Syntax *addition_new(Syntax *left, Syntax *right);

Syntax *multiplication_new(Syntax *left, Syntax *right);

Syntax *less_than_new(Syntax *left, Syntax *right);

Syntax *function_call_new(char *function_name, Syntax *func_args);

Syntax *assignment_new(char *var_name, Syntax *expression);

Syntax *return_statement_new(Syntax *expression);

Syntax *block_new(List *statements);

Syntax *if_new(Syntax *condition, Syntax *then);

Syntax *define_var_new(char *var_name, Syntax *init_value);

Syntax *while_new(Syntax *condition, Syntax *body);

Syntax *function_new(char *name, List *parameters, Syntax *root_block);

Syntax *top_level_new();

void syntax_free(Syntax *syntax);

char *syntax_type_name(Syntax *syntax);

void print_syntax(Syntax *syntax);

#endif
