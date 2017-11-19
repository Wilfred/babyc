
/* ----------------------------------------------------------------
 *
 * BabyC Toy compiler for educational purposes
 *
 * ---------------------------------------------------------------- */

#ifndef BABYC_SYNTAX_HEADER
#define BABYC_SYNTAX_HEADER

#include "list.h"
#include <stdbool.h>

typedef enum {
    IMMEDIATE,
    VARIABLE,
    ADDRESS,
    READ_ADDRESS,
    WRITE_ADDRESS,
    UNARY_OPERATOR,
    BINARY_OPERATOR,
    BLOCK,
    LABEL_STATEMENT,
    GOTO_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    NOP_STATEMENT,
    IF_STATEMENT,
    RETURN_STATEMENT,
    ASSIGNMENT_STATEMENT,
    ASSIGNMENT_STATIC,
    WHILE_STATEMENT,
    FUNCTION_PARAMETER,
    FUNCTION_DEFINITION,
    FUNCTION_ARGUMENT,
    FUNCTION_CALL,
} SyntaxType;

typedef enum {
    ARITHMETIC_NEGATION,
    BITWISE_NEGATION,
    LOGICAL_NEGATION,
} UnaryExpressionType;

typedef enum {
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    MODULUS,
    AND,
    OR,
    XOR,
    RSHIFT,
    LSHIFT,
    LOGICAL_AND,
    LOGICAL_OR,
    LESS_THAN,
    LESS_THAN_OR_EQUAL,
    LARGER_THAN,
    LARGER_THAN_OR_EQUAL,
    EQUAL,
    NEQUAL,
} BinaryExpressionType;

typedef enum {
    GLOBAL,
    PARAMETER,
    AUTOMATIC,
    REGISTER, /* TODO promote some variables to register , at least inside leaf
                 functions (no need to save and restore)  */
} Storage;

typedef enum {
    O_UNDEF = 0,
    O_UNSIGNED = 0x001,
    O_ADDRESS = 0x200,
    O_VOID = 0x0100,
    O_UINT1 = 0x0003,
    O_BOOL = 0x0003,
    O_INT8 = 0x0004,
    O_INT16 = 0x0008,
    O_INT32 = 0x0010,
    O_INT64 = 0x0020,
    O_INT128 = 0x0040,
    O_UINT8 = 0x0005,
    O_UINT16 = 0x0009,
    O_UINT32 = 0x0011,
    O_UINT64 = 0x0021,
    O_UINT128 = 0x0041,
} ObjectType;

typedef struct Variable {
    char *name;      /* variable name */
    Storage storage; /* where the variable is stored */
    int offset;      /* offset in storage */
    char *assembler_name;
    ObjectType objectType;
} Variable;

typedef struct Scope {
    struct Scope *prev_scope;
    List *var_stack;
    List *parser_stack;
    List *arguments_stack;
} Scope;

Scope *scope_new(Scope *current);
Scope *scope_del(Scope *current);
Variable *scope_get_var(Scope *ctx, char *name);
Variable *scope_add_var(Scope *ctx, char *name, ObjectType type,
                        Storage storage);

struct Syntax;
typedef struct Syntax Syntax;

typedef struct Immediate { int value; } Immediate;

typedef struct Label {
    char *name;
    char *assembler_name;
} Label;

typedef struct UnaryExpression {
    UnaryExpressionType unary_type;
    Syntax *expression;
    ObjectType objectType;
} UnaryExpression;

typedef struct BinaryExpression {
    BinaryExpressionType binary_type;
    Syntax *left;
    Syntax *right;
    ObjectType objectType;
} BinaryExpression;

typedef struct AddressExpression {
    Syntax *identifier;
    Syntax *offset;
} AddressExpression;

typedef struct ReadAddressExpression {
    Syntax *address;
    Syntax *offset;
    ObjectType objectType;
} ReadAddressExpression;

typedef struct WriteAddressExpression {
    Syntax *address;
    Syntax *offset;
    Syntax *expression;
    ObjectType objectType;
} WriteAddressExpression;

typedef struct FunctionArgument { Syntax *expression; } FunctionArgument;

typedef struct FunctionCall {
    char *name;
    List *arguments;
} FunctionCall;

typedef struct FunctionParameter { Variable *variable; } FunctionParameter;

typedef struct FunctionDefinition {
    char *name;
    List *parameters;
    List *labels;
    Syntax *block;
    int max_automatic_offset;
    int max_dynamic_offset;
    ObjectType type;
} FunctionDefinition;

typedef struct Assignment {
    Variable *variable;
    Syntax *expression;
} Assignment;

typedef struct IfStatement {
    Syntax *condition;
    Syntax *if_then;
    Syntax *if_else;
} IfStatement;

typedef struct DefineVarStatement { 
    char *var_name; 
} DefineVarStatement;

typedef struct WhileStatement {
    Syntax *condition;
    Syntax *body;
} WhileStatement;

typedef struct ReturnStatement { Syntax *expression; } ReturnStatement;

typedef struct Block { List *statements; } Block;

typedef struct TopLevel { List *declarations; } TopLevel;

struct Syntax {
    SyntaxType type;
    union {
        Immediate *immediate;

        Variable *variable;

        Label *label;

        AddressExpression *address;

        ReadAddressExpression *read_address;

        WriteAddressExpression *write_address;

        UnaryExpression *unary_expression;

        BinaryExpression *binary_expression;

        Assignment *assignment;

        ReturnStatement *return_statement;

        IfStatement *if_statement;

        DefineVarStatement *define_var_statement;

        WhileStatement *while_statement;

        Block *block;

        FunctionArgument *function_argument;

        FunctionCall *function_call;

        FunctionParameter *function_parameter;

        FunctionDefinition *function_definition;
    };
};

Syntax *nop_new(void);

Syntax *label_statement_new(Label *l);

Syntax *goto_statement_new(Label *l);

Syntax *immediate_new(char *value);

Syntax *variable_new(Variable *v);

Syntax *address_new(Syntax *expression, Syntax *offset);

Syntax *read_pointer_new(Syntax *address, Syntax *offset);

Syntax *write_pointer_new(Syntax *address, Syntax *offset, Syntax *expression);

Syntax *bitwise_negation_new(Syntax *expression);

Syntax *arithmetic_negation_new(Syntax *expression);

Syntax *logical_negation_new(Syntax *expression);

Syntax *addition_new(Syntax *left, Syntax *right);

Syntax *subtraction_new(Syntax *left, Syntax *right);

Syntax *multiplication_new(Syntax *left, Syntax *right);

Syntax *division_new(Syntax *left, Syntax *right);

Syntax *modulus_new(Syntax *left, Syntax *right);

Syntax *bitwise_xor_new(Syntax *left, Syntax *right);

Syntax *bitwise_or_new(Syntax *left, Syntax *right);

Syntax *bitwise_and_new(Syntax *left, Syntax *right);

Syntax *logical_or_new(Syntax *left, Syntax *right);

Syntax *logical_and_new(Syntax *left, Syntax *right);

Syntax *rshift_new(Syntax *left, Syntax *right);

Syntax *lshift_new(Syntax *left, Syntax *right);

Syntax *less_than_new(Syntax *left, Syntax *right);

Syntax *less_or_equal_new(Syntax *left, Syntax *right);

Syntax *larger_than_new(Syntax *left, Syntax *right);

Syntax *larger_or_equal_new(Syntax *left, Syntax *right);

Syntax *equal_new(Syntax *left, Syntax *right);

Syntax *nequal_new(Syntax *left, Syntax *right);

Syntax *function_call_new(char *function_name, List *args);

Syntax *function_argument_new(Syntax *s);

Syntax *function_definition_new(char *name, ObjectType type, List *parms,
                                List *labels, Syntax *block);

Syntax *function_parameter_new(Variable *v);

Syntax *assignment_new(Variable *v, Syntax *expression);

Syntax *assignment_static_new(Variable *v, Syntax *expression);

Syntax *return_statement_new(Syntax *expression);

Syntax *break_statement_new(void);

Syntax *continue_statement_new(void);

Syntax *block_new(List *statements);

Syntax *if_new(Syntax *condition, Syntax *if_then, Syntax *if_else);

Syntax *while_new(Syntax *condition, Syntax *body);

Syntax *object_type_size_syntax(ObjectType type);

int object_type_size_value(ObjectType type);

ObjectType convert_type(char *s);

bool syntax_is_boolean(Syntax *syntax);

void syntax_free(Syntax *syntax);

const char *syntax_type_name(Syntax *syntax);
const char *syntax_type_type(ObjectType type);
ObjectType ast_annotate_syntax(Syntax *syntax, ObjectType dest);

void print_syntax_item(Syntax *syntax);
void print_syntax_list(List *list);

void parser_setup(char *file_name);

Syntax *parser_complete(void);

#endif
