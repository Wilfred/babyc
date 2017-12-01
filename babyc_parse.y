%file-prefix "bb"
%name-prefix "bb"

%{

/* ----------------------------------------------------------------
 * 
 * BabyC Toy compiler for educational purposes
 * 
 * ---------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "syntax.h"
#include "list.h"
#include "bb_type.h"
#include "log_error.h"

#define YYDEBUG 1
#undef YYDEBUG
// int yydebug=1;

extern int bblex(void);

int bberror(char *s)
{
    printf("%s", s);
    return 1;
}

int bbwrap()
{
	return 1;
}

extern FILE *bbin;

static Scope *pscope;
static List *parameters_stack;
static List *labels_stack;

void parser_setup(char *file_name)
{
    pscope = scope_new(0);
    parameters_stack = list_new();
    labels_stack = list_new();
    bbin = fopen(file_name, "rt");
}

Syntax *parser_complete(void)
{
    Syntax *ret = block_new(pscope->parser_stack);
    list_free(parameters_stack);
    list_free(labels_stack);
    fclose(bbin);
    return ret; 
}

Label *search_existing_label(char *name)
{
   for (int i = 0; i < list_length(labels_stack); i++)
   {
       Label *l = list_get(labels_stack, i);
       if (!strcmp(name, l->name))
       {
           return l;
       }
   }
   Label *label = malloc(sizeof(Label));
   label->name = name;
   label->assembler_name = 0;
   list_push(labels_stack, label);
   return label;
}
 
static ObjectType current_object_type = O_INT32;
static ObjectType current_function_type = O_INT32;
static ObjectType current_cast_type = O_INT32;
static ObjectType saved_object_type = O_INT32; 
%}

%define parse.trace

%token T_STATIC T_TYPE T_UNSIGNED T_SIGNED T_VOID 
%token T_STRING T_IDENTIFIER T_RETURN T_NUMBER
%token T_OPEN_BRACE T_CLOSE_BRACE
%token T_IF T_WHILE T_ELSE T_GOTO T_LABEL T_BREAK T_CONTINUE
%token T_LSHIFT T_RSHIFT T_LOGICAL_OR T_LOGICAL_AND
%token T_LESS_OR_EQUAL T_EQUAL T_NEQUAL T_LARGER_OR_EQUAL
%token T_INCREMENT T_DECREMENT T_SIZEOF
%token T_PLUS_EQ T_MINUS_EQ T_MUL_EQ T_DIV_EQ T_MOD_EQ T_OR_EQ T_AND_EQ T_XOR_EQ T_RSHIFT_EQ T_LSHIFT_EQ

/* Operator associativity, least precedence first.
 * See http://en.cppreference.com/w/c/language/operator_precedence
 */
%left '=' T_PLUS_EQ T_MINUS_EQ T_MUL_EQ T_DIV_EQ T_MOD_EQ T_OR_EQ T_AND_EQ T_RSHIFT_EQ T_LSHIFT_EQ
%left T_LOGICAL_OR
%left T_LOGICAL_AND
%left '&' '|' '^'
%left T_EQUAL T_NEQUAL
%left '<' '>' T_LARGER_OR_EQUAL T_LESS_OR_EQUAL
%left T_LSHIFT T_RSHIFT
%left '+' '-' 
%left '*' '/' '%'
%precedence T_INCREMENT T_DICREMENT
%precedence T_MINUS T_PLUS
%nonassoc '!'
%nonassoc '~'
%precedence T_ADDRESS

%%

program:
        top_element program
        |
        T_STATIC top_element program
        |
        ;

object_type:
        basic_object_type
        |
        basic_object_type '*' 
        {
            current_object_type = current_object_type | O_ADDRESS;
        }
        ;
        
basic_object_type:
        T_TYPE
        {
            saved_object_type = current_object_type;
            current_object_type = convert_type($1.symbol);
        }
        |
        T_VOID
        {
            saved_object_type = current_object_type;
            current_object_type = O_VOID;
        }
        |
        T_SIGNED T_TYPE
        {
            saved_object_type = current_object_type;
            current_object_type = convert_type($2.symbol) & ~O_UNSIGNED;   
        }
        |
        T_UNSIGNED T_TYPE
        {
            saved_object_type = current_object_type;
            current_object_type = convert_type($2.symbol) | O_UNSIGNED;   
        }
        ;

top_element:
	    object_type T_IDENTIFIER '('  { current_function_type = current_object_type; pscope = scope_new(pscope); } 
        parameter_list ')' T_OPEN_BRACE full_block T_CLOSE_BRACE
        {
            Syntax *block = block_new(pscope->parser_stack);
            Syntax *syntax = function_definition_new($2.symbol, current_function_type, parameters_stack, labels_stack, block);
            pscope->parser_stack = list_new();
            parameters_stack = list_new();
            labels_stack = list_new();
            pscope = scope_del(pscope);   
            list_push(pscope->parser_stack, syntax);
        }
        |
        object_type global_identifier_list ';'
        ;

global_identifier_list:
        global_identifier ',' global_identifier_list
        | 
        global_identifier
        ;

global_identifier: 
        T_IDENTIFIER '=' expression
        {
            Syntax *init = list_pop(pscope->parser_stack);
            Variable *v = scope_add_var(pscope, $1.symbol, current_object_type, GLOBAL);
            list_push(pscope->parser_stack, assignment_static_new(v, init));
        }
        |
        T_IDENTIFIER
        {
            Variable *v = scope_add_var(pscope, $1.symbol, current_object_type, GLOBAL);
            list_push(pscope->parser_stack, assignment_static_new(v, 0));
        }
        ;

automatic_identifier_list:
        automatic_identifier ',' automatic_identifier_list
        | 
        automatic_identifier
        ;

automatic_identifier: 
        T_IDENTIFIER '=' expression
        {
            Syntax *init = list_pop(pscope->parser_stack);
            Variable *v = scope_add_var(pscope, $1.symbol, current_object_type, AUTOMATIC);
            list_push(pscope->parser_stack, assignment_new(v, init));
        }
        |
        T_IDENTIFIER
        {
            scope_add_var(pscope, $1.symbol, current_object_type, AUTOMATIC);
        }
        ;

parameter_list:
        nonempty_parameter_list
        |
        ;

nonempty_parameter_list:
        parameter ',' parameter_list
        |
        parameter 
        ;

parameter: object_type T_IDENTIFIER
        {
            Variable *v = scope_add_var(pscope, $2.symbol, current_object_type, PARAMETER);
            list_push(parameters_stack, function_parameter_new(v));
        }
        ;

argument_list:
        nonempty_argument_list
        |
        ;

nonempty_argument_list:
        argument ',' nonempty_argument_list
        |
        argument
        ;

argument:
        expression
        {
            Syntax *argument = list_pop(pscope->parser_stack);
            list_push(pscope->arguments_stack, function_argument_new(argument));
        }
        ;

block: 
        T_OPEN_BRACE { pscope = scope_new(pscope); } 
        full_block 
        T_CLOSE_BRACE  
        { 
            Syntax *syntax = block_new(pscope->parser_stack);
            pscope->parser_stack = list_new();
            pscope = scope_del(pscope);   
            list_push(pscope->parser_stack, syntax);
        }
        ;

full_block:
        nonempty_statement_list
        |
        ;

nonempty_statement_list:
        statement nonempty_statement_list
        |
        statement
        ;

statement:
        T_GOTO T_IDENTIFIER ';'
        {
            Label *label = search_existing_label($2.symbol);
            list_push(pscope->parser_stack, goto_statement_new(label));
        }
        |
        T_BREAK ';'
        {
            list_push(pscope->parser_stack, break_statement_new());
        }
        |
        T_CONTINUE ';'
        {
            list_push(pscope->parser_stack, continue_statement_new());
        }
        |
        T_LABEL ':'
        {
            Label *label = search_existing_label($1.symbol);
            list_push(pscope->parser_stack, label_statement_new(label));
        }
        |
        T_RETURN expression ';'
        {
            Syntax *current_syntax = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, return_statement_new(current_syntax, current_function_type));
        }
        |
        T_IF '(' expression ')' statement
        {
            Syntax *if_else = nop_new();
            Syntax *if_then = list_pop(pscope->parser_stack);
            Syntax *condition = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, if_new(condition, if_then, if_else));
        }
        |
        T_IF '(' expression ')' statement T_ELSE statement
        {
            Syntax *if_else = list_pop(pscope->parser_stack);
            Syntax *if_then = list_pop(pscope->parser_stack);
            Syntax *condition = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, if_new(condition, if_then, if_else));
        }
        |
        T_WHILE '(' expression ')' statement
        {
            Syntax *body = list_pop(pscope->parser_stack);
            Syntax *condition = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, while_new(condition, body));
        }
        |
        object_type automatic_identifier_list ';'
        |
        T_STATIC object_type global_identifier_list ';'
        |
        expression ';'
        | 
        block
        ;

address:
        T_IDENTIFIER
        {
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, variable_new(v));            
        }
        | '(' expression ')'
        {
            
        }
        ;
        
expression:
	    T_NUMBER
        {
            list_push(pscope->parser_stack, immediate_new($1.symbol));
        }
        |
	    T_IDENTIFIER
        {
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, variable_new(v));
        }
        |
	    '&' T_IDENTIFIER
        {
            Variable *v = scope_get_var(pscope, $2.symbol);
            list_push(pscope->parser_stack, address_new(variable_new(v), immediate_new("0")));
        }
        |
	    '&' T_IDENTIFIER '[' expression ']'
        {
            Variable *v = scope_get_var(pscope, $2.symbol);
            Syntax *offset = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, address_new(variable_new(v), offset));
        }
        |
	    address '[' expression ']'
        {
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, read_pointer_new(address, offset));
        }
        |
	    '*' address
        {
            Syntax *offset = immediate_new("0");
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, read_pointer_new(address, offset));
        }
        |
        T_IDENTIFIER '=' expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, assignment_new(v, expression));
        }
        |
        T_IDENTIFIER T_PLUS_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, assignment_new(v, addition_new(variable_new(v), expression)));
        }
        |
        T_IDENTIFIER T_MINUS_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, assignment_new(v, subtraction_new(variable_new(v), expression)));
        }
        |
        T_IDENTIFIER T_MUL_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, assignment_new(v, multiplication_new(variable_new(v), expression)));
        }
        |
        T_IDENTIFIER T_DIV_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, assignment_new(v, division_new(variable_new(v), expression)));
        }
        |
        T_IDENTIFIER T_MOD_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, assignment_new(v, modulus_new(variable_new(v), expression)));
        }
        |
        T_IDENTIFIER T_AND_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, assignment_new(v, bitwise_and_new(variable_new(v), expression)));
        }
        |
        T_IDENTIFIER T_OR_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, assignment_new(v, bitwise_or_new(variable_new(v), expression)));
        }
        |
        T_IDENTIFIER T_XOR_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, assignment_new(v, bitwise_xor_new(variable_new(v), expression)));
        }
        |
        T_IDENTIFIER T_LSHIFT_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, assignment_new(v, lshift_new(variable_new(v), expression)));
        }
        |
        T_IDENTIFIER T_RSHIFT_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, $1.symbol);
            list_push(pscope->parser_stack, assignment_new(v, rshift_new(variable_new(v), expression)));
        }
        |
	    address '[' expression ']' '=' expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, expression));
        }
        |
	    address '[' expression ']' T_PLUS_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, addition_new(read_pointer_new(address, offset), expression)));
        }
        |
	    address '[' expression ']' T_MINUS_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, subtraction_new(read_pointer_new(address, offset), expression)));
        }
        |
	    address '[' expression ']' T_MUL_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, multiplication_new(read_pointer_new(address, offset), expression)));
        }
        |
	    address '[' expression ']' T_DIV_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, division_new(read_pointer_new(address, offset), expression)));
        }
        |
	    address '[' expression ']' T_MOD_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, modulus_new(read_pointer_new(address, offset), expression)));
        }
        |
	    address '[' expression ']' T_LSHIFT_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, lshift_new(read_pointer_new(address, offset), expression)));
        }
        |
	    address '[' expression ']' T_RSHIFT_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, rshift_new(read_pointer_new(address, offset), expression)));
        }
        |
	    address '[' expression ']' T_AND_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, bitwise_and_new(read_pointer_new(address, offset), expression)));
        }
        |
	    address '[' expression ']' T_OR_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, bitwise_or_new(read_pointer_new(address, offset), expression)));
        }
        |
	    address '[' expression ']' T_XOR_EQ expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = list_pop(pscope->parser_stack);
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, bitwise_xor_new(read_pointer_new(address, offset), expression)));
        }
        |
	    '*' address '=' expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Syntax *offset = immediate_new("0");
            Syntax *address = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, write_pointer_new(address, offset, expression));
        }
        |
        '~' expression
        {
            Syntax *current_syntax = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, bitwise_negation_new(current_syntax));
        }
        |
        '!' expression
        {
            Syntax *current_syntax = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, logical_negation_new(current_syntax));
        }
        |
        '-' expression %prec T_MINUS
        {
            Syntax *current_syntax = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, arithmetic_negation_new(current_syntax));
        }
        |
        '+' expression %prec T_PLUS
        {
            /* already in AST */
        }
        |
        expression '+' expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, addition_new(left, right));
        }
        |
        expression '-' expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, subtraction_new(left, right));
        }
        |
        expression '*' expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, multiplication_new(left, right));
        }
        |
        expression '/' expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, division_new(left, right));
        }
        |
        expression '%' expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, modulus_new(left, right));
        }
        |
        expression '&' expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, bitwise_and_new(left, right));
        }
        |
        expression '|' expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, bitwise_or_new(left, right));
        }
        |
        expression '^' expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, bitwise_xor_new(left, right));
        }
        |
        expression T_RSHIFT expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, rshift_new(left, right));
        }
        |
        expression T_LSHIFT expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, lshift_new(left, right));
        }
        |
        expression T_LOGICAL_OR expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, logical_or_new(left, right));
        }
        |
        expression T_LOGICAL_AND expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, logical_and_new(left, right));
        }
        |
        expression '<' expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, less_than_new(left, right));
        }
        |
        expression '>' expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, larger_than_new(left, right));
        }
        |
        expression T_EQUAL expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, equal_new(left, right));
        }
        |
        expression T_NEQUAL expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, nequal_new(left, right));
        }
        |
        expression T_LARGER_OR_EQUAL expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, larger_or_equal_new(left, right));
        }
        |
        expression T_LESS_OR_EQUAL expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, less_or_equal_new(left, right));
        }
        |
        T_SIZEOF '(' object_type ')'
        {
            list_push(pscope->parser_stack, object_type_size_syntax(current_object_type));
            current_object_type = saved_object_type;
        }
        |
        '(' object_type { current_cast_type = current_object_type;  current_object_type = saved_object_type; } ')' expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, cast_new(current_cast_type, expression));
           
        }
        |
        T_SIZEOF '(' T_IDENTIFIER ')'
        {
            Variable *v = scope_get_var(pscope, $3.symbol);
            if (!v) {
                log_error("Should only get sizeof(%s) from valid variables", $3.symbol);
            }
            list_push(pscope->parser_stack, object_type_size_syntax(v->objectType));
        }
        |
        T_IDENTIFIER '(' { pscope = scope_new(pscope); } argument_list ')'
        {
            List *l = list_swap(pscope->arguments_stack);
            pscope->arguments_stack = list_new();
            pscope = scope_del(pscope);   
            list_push(pscope->parser_stack, function_call_new($1.symbol, l));
        }
        |
        '(' expression ')'
        |
        '(' block ')'
        ;
