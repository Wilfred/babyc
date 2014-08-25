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

Syntax *variable_new(char *var_name) {
    Variable *variable = malloc(sizeof(Variable));
    variable->var_name = var_name;
    
    Syntax *syntax = malloc(sizeof(Syntax));
    syntax->type = VARIABLE;
    syntax->variable = variable;

    return syntax;
}

Syntax *bitwise_negation_new(Syntax *expression) {
    Syntax *syntax = malloc(sizeof(Syntax));

    UnaryExpression *unary_syntax = malloc(sizeof(UnaryExpression));

    unary_syntax->unary_type = BITWISE_NEGATION;
    unary_syntax->expression = expression;

    syntax->type = UNARY_OPERATOR;
    syntax->unary_expression = unary_syntax;

    return syntax;
}

Syntax *logical_negation_new(Syntax *expression) {
    Syntax *syntax = malloc(sizeof(Syntax));

    UnaryExpression *unary_syntax = malloc(sizeof(UnaryExpression));

    unary_syntax->unary_type = LOGICAL_NEGATION;
    unary_syntax->expression = expression;

    syntax->type = UNARY_OPERATOR;
    syntax->unary_expression = unary_syntax;

    return syntax;
}

Syntax *addition_new(Syntax *left, Syntax *right) {
    Syntax *syntax = malloc(sizeof(Syntax));

    BinaryExpression *binary_syntax = malloc(sizeof(BinaryExpression));

    binary_syntax->binary_type = ADDITION;
    binary_syntax->left = left;
    binary_syntax->right = right;

    syntax->type = BINARY_OPERATOR;
    syntax->binary_expression = binary_syntax;

    return syntax;
}

Syntax *multiplication_new(Syntax *left, Syntax *right) {
    Syntax *syntax = malloc(sizeof(Syntax));

    BinaryExpression *binary_syntax = malloc(sizeof(BinaryExpression));

    binary_syntax->binary_type = MULTIPLICATION;
    binary_syntax->left = left;
    binary_syntax->right = right;

    syntax->type = BINARY_OPERATOR;
    syntax->binary_expression = binary_syntax;

    return syntax;
}

Syntax *assignment_new(char *var_name, Syntax *expression) {
    Assignment *assignment = malloc(sizeof(Assignment));
    assignment->var_name = var_name;
    assignment->expression = expression;

    Syntax *syntax = malloc(sizeof(Syntax));
    syntax->type = ASSIGNMENT;
    syntax->assignment = assignment;

    return syntax;
}

Syntax *return_statement_new(Syntax *expression) {
    Syntax *syntax = malloc(sizeof(Syntax));

    Statement *statement = malloc(sizeof(Statement));

    statement->statement_type = RETURN_STATEMENT;
    statement->return_expression = expression;

    syntax->type = STATEMENT;
    syntax->statement = statement;

    return syntax;
}

Syntax *block_new(List *statements) {
    Syntax *syntax = malloc(sizeof(Syntax));

    Block *block = malloc(sizeof(Block));

    block->statements = statements;

    syntax->type = BLOCK;
    syntax->block = block;

    return syntax;
}

Syntax *if_new(Syntax *condition, Syntax *then) {
    Syntax *syntax = malloc(sizeof(Syntax));

    IfStatement *if_statement = malloc(sizeof(IfStatement));

    if_statement->condition = condition;
    if_statement->then = then;

    syntax->type = IF_STATEMENT_SYNTAX;
    syntax->if_statement = if_statement;

    return syntax;
}

Syntax *define_var_new(char *var_name, Syntax *init_value) {
    Syntax *syntax = malloc(sizeof(Syntax));

    DefineVarStatement *define_var_statement = malloc(sizeof(DefineVarStatement));
    define_var_statement->var_name = var_name;
    define_var_statement->init_value = init_value;

    syntax->type = DEFINE_VAR_SYNTAX;
    syntax->define_var_statement = define_var_statement;

    return syntax;
}

Syntax *function_new(Syntax *root_block) {
    Syntax *syntax = malloc(sizeof(Syntax));

    Function *function = malloc(sizeof(Function));

    function->root_block = root_block;

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
        Statement *statement = syntax->statement;
        if (statement->statement_type == RETURN_STATEMENT) {
            syntax_free(statement->return_expression);
        } else if (statement->statement_type == IF_STATEMENT) {
            syntax_free(statement->if_statement);
        }
        
    } else if (syntax->type == IF_STATEMENT_SYNTAX) {
        syntax_free(syntax->if_statement->condition);
        syntax_free(syntax->if_statement->then);
        
    } else if (syntax->type == BLOCK) {
        list_free(syntax->block->statements);
        
    } else if (syntax->type == FUNCTION) {
        free(syntax->block);
    }
    free(syntax);
}

char *syntax_type_name(Syntax *syntax) {
    if (syntax->type == IMMEDIATE) {
        return "IMMEDIATE";
    } else if (syntax->type == VARIABLE) {
        return "VARIABLE";
    } else if (syntax->type == UNARY_OPERATOR) {
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
    } else if (syntax->type == STATEMENT) {
        if (syntax->statement->statement_type == RETURN_STATEMENT) {
            return "STATEMENT RETURN";
        } else if (syntax->statement->statement_type == IF_STATEMENT) {
            return "STATEMENT IF";
        }
    } else if (syntax->type == IF_STATEMENT_SYNTAX) {
        return "IF";
    } else if (syntax->type == DEFINE_VAR_SYNTAX) {
        return "DEFINE VARIABLE";
    } else if (syntax->type == BLOCK) {
        return "BLOCK";
    } else if (syntax->type == FUNCTION) {
        return "FUNCTION";
    } else if (syntax->type == ASSIGNMENT) {
        return "ASSIGNMENT";
    }

    // Should never be reached.
    return "??? UNKNOWN SYNTAX";
}

void print_syntax_indented(Syntax *syntax, int indent) {
    for (int i=0; i<indent; i++) {
        printf(" ");
    }

    char *syntax_type_string = syntax_type_name(syntax);

    if (syntax->type == IMMEDIATE) {
        printf("%s %d\n", syntax_type_string, syntax->value);
    } else if (syntax->type == VARIABLE) {
        printf("%s '%s'\n", syntax_type_string, syntax->variable->var_name);
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
        
    } else if (syntax->type == STATEMENT) {
        printf("%s\n", syntax_type_string);
        if (syntax->statement->statement_type == RETURN_STATEMENT) {
            print_syntax_indented(syntax->statement->return_expression, indent + 4);
        } else if (syntax->statement->statement_type == IF_STATEMENT) {
            print_syntax_indented(syntax->statement->if_statement, indent + 4);
        }

    } else if (syntax->type == IF_STATEMENT_SYNTAX) {
        printf("%s CONDITION\n", syntax_type_string);
        print_syntax_indented(syntax->if_statement->condition, indent + 4);

        for (int i=0; i<indent; i++) {
            printf(" ");
        }

        printf("%s THEN\n", syntax_type_string);
        print_syntax_indented(syntax->if_statement->then, indent + 4);

    } else if (syntax->type == DEFINE_VAR_SYNTAX) {
        printf("%s '%s'\n", syntax_type_string, syntax->define_var_statement->var_name);

        for (int i=0; i<indent; i++) {
            printf(" ");
        }

        printf("'%s' INITIAL VALUE\n", syntax->define_var_statement->var_name);
        print_syntax_indented(syntax->define_var_statement->init_value, indent + 4);

    } else if (syntax->type == BLOCK) {
        printf("%s\n", syntax_type_string);

        List *statements = syntax->block->statements;
        for (int i=0; i<list_length(statements); i++) {
            print_syntax_indented(list_get(statements, i), indent + 4);
        }

    } else if (syntax->type == FUNCTION) {
        printf("%s\n", syntax_type_string);
        print_syntax_indented(syntax->function->root_block, indent + 4);
        
    } else if (syntax->type == ASSIGNMENT) {
        printf("%s '%s'\n", syntax_type_string, syntax->assignment->var_name);
        print_syntax_indented(syntax->assignment->expression, indent + 4);
        
    } else {
        printf("??? UNKNOWN SYNTAX TYPE\n");
    }
}

void print_syntax(Syntax *syntax) {
    print_syntax_indented(syntax, 0);
}
