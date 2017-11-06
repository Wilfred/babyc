%{

/* ----------------------------------------------------------------
 * 
 * Brave Algorithms Build Your Code
 * 
 * ---------------------------------------------------------------- */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "syntax.h"
#include "list.h"

#define YYSTYPE char*
#define YYDEBUG 1

int yyparse(void);
int yylex();

void yyerror(const char *str)
{
	fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
	return 1;
}

// int yydebug=1;

extern FILE *yyin;

static Scope *pscope;
static List *parameters_stack;
static List *labels_stack;

void parser_setup(char *file_name)
{
    pscope = scope_new(0);
    parameters_stack = list_new();
    labels_stack = list_new();
    yyin = fopen(file_name, "rt");
}

Syntax *parser_complete(void)
{
    Syntax *ret = block_new(pscope->parser_stack);
    list_free(parameters_stack);
    list_free(labels_stack);
    fclose(yyin);
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
 
%}

%define parse.trace

%token T_INCLUDE T_HEADER_NAME
%token T_STATIC T_TYPE T_STRING T_IDENTIFIER T_RETURN T_NUMBER
%token T_OPEN_BRACE T_CLOSE_BRACE
%token T_IF T_WHILE T_ELSE T_GOTO T_LABEL
%token T_LSHIFT T_RSHIFT
%token T_LESS_OR_EQUAL T_EQUAL T_NEQUAL T_LARGER_OR_EQUAL

/* Operator associativity, least precedence first.
 * See http://en.cppreference.com/w/c/language/operator_precedence
 */
%left '='
%left '&' '|' '^'
%left T_EQUAL T_NEQUAL
%left '<' '>' T_LARGER_OR_EQUAL T_LESS_OR_EQUAL
%left T_LSHIFT T_RSHIFT
%left '+' '-' 
%left '*' '/' '%'
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

top_element:
	    T_TYPE T_IDENTIFIER '('  { pscope = scope_new(pscope); } 
        parameter_list ')' T_OPEN_BRACE full_block T_CLOSE_BRACE
        {
            Syntax *block = block_new(pscope->parser_stack);
            Syntax *syntax = function_definition_new((char*)$2, parameters_stack, labels_stack, block);
            pscope->parser_stack = list_new();
            parameters_stack = list_new();
            labels_stack = list_new();
            pscope = scope_del(pscope);   
            list_push(pscope->parser_stack, syntax);
        }
        |
        T_TYPE T_IDENTIFIER '=' expression ';'
        {
            Syntax *init = list_pop(pscope->parser_stack);
            Variable *v = scope_add_var(pscope, (char*)$2, GLOBAL);
            list_push(pscope->parser_stack, assignment_static_new(v, init));
        }
        |
        T_TYPE T_IDENTIFIER ';'
        {
            Variable *v = scope_add_var(pscope, (char*)$2, GLOBAL);
            list_push(pscope->parser_stack, assignment_static_new(v, 0));
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

parameter: T_TYPE T_IDENTIFIER
        {
            Variable *v = scope_add_var(pscope, (char*)$2, PARAMETER);
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
            Label *label = search_existing_label((char *)$2);
            list_push(pscope->parser_stack, goto_statement_new(label));
        }
        |
        T_LABEL ':'
        {
            Label *label = search_existing_label((char *)$1);
            list_push(pscope->parser_stack, label_statement_new(label));
        }
        |
        T_RETURN expression ';'
        {
            Syntax *current_syntax = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, return_statement_new(current_syntax));
        }
        |
        T_IF '(' expression ')' block
        {
            // TODO: else statements.
            Syntax *if_then = list_pop(pscope->parser_stack);
            Syntax *condition = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, if_new(condition, if_then, nop_new()));
        }
        |
        T_IF '(' expression ')' block T_ELSE block
        {
            // TODO: else statements.
            Syntax *if_else = list_pop(pscope->parser_stack);
            Syntax *if_then = list_pop(pscope->parser_stack);
            Syntax *condition = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, if_new(condition, if_then, if_else));
        }
        |
        T_WHILE '(' expression ')' block
        {
            Syntax *body = list_pop(pscope->parser_stack);
            Syntax *condition = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, while_new(condition, body));
        }
        |
        T_TYPE T_IDENTIFIER '=' expression ';'
        {
            Syntax *init_syntax = list_pop(pscope->parser_stack);
            Variable *v = scope_add_var(pscope, (char*)$2, AUTOMATIC);
            list_push(pscope->parser_stack, assignment_new(v, init_syntax));
        }
        |
        T_STATIC T_TYPE T_IDENTIFIER '=' expression ';'
        {
            Syntax *init_syntax = list_pop(pscope->parser_stack);
            Variable *v = scope_add_var(pscope, (char*)$3, GLOBAL);
            list_push(pscope->parser_stack, assignment_static_new(v, init_syntax));
        }
        |
        T_TYPE T_IDENTIFIER ';'
        {
            scope_add_var(pscope, (char*)$2, AUTOMATIC);
        }
        |
        T_STATIC T_TYPE T_IDENTIFIER ';'
        {
            Variable *v = scope_add_var(pscope, (char*)$3, GLOBAL);
            list_push(pscope->parser_stack, assignment_static_new(v, 0));
        }
        |
        expression ';'
        {
            // Nothing to do, we have the AST node already.
        }
        ;

expression:
	    T_NUMBER
        {
            list_push(pscope->parser_stack, immediate_new((char*)$1));
        }
        |
	    T_IDENTIFIER
        {
            Variable *v = scope_get_var(pscope, (char *)$1);
            list_push(pscope->parser_stack, variable_new(v));
        }
        |
	    '&' T_IDENTIFIER
        {
            Variable *v = scope_get_var(pscope, (char *)$2);
            list_push(pscope->parser_stack, address_new(v));
        }
        |
        T_IDENTIFIER '=' expression
        {
            Syntax *expression = list_pop(pscope->parser_stack);
            Variable *v = scope_get_var(pscope, (char *)$1);
            list_push(pscope->parser_stack, assignment_new(v, expression));
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
            list_push(pscope->parser_stack, less_than_new(right, left));
        }
        |
        expression T_EQUAL expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, equal_new(right, left));
        }
        |
        expression T_NEQUAL expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, nequal_new(right, left));
        }
        |
        expression T_LARGER_OR_EQUAL expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, less_or_equal_new(right, left));
        }
        |
        expression T_LESS_OR_EQUAL expression
        {
            Syntax *right = list_pop(pscope->parser_stack);
            Syntax *left = list_pop(pscope->parser_stack);
            list_push(pscope->parser_stack, less_or_equal_new(left, right));
        }
        |
        T_IDENTIFIER '(' { pscope = scope_new(pscope); } argument_list ')'
        {
            if (!strcmp($2, "sizeof")) {
                Syntax *syntax = list_pop(pscope->arguments_stack);
                if (syntax->type != FUNCTION_ARGUMENT) {
                    printf("Should only get the sizeof(variable)\n");
                    assert(0);
                }
                FunctionArgument *f = syntax->function_argument;
                syntax = f->expression;
                if (syntax->type != VARIABLE) {
                    printf("Should only get the sizeof(variable)\n");
                    assert(0);
                }
                Variable *v = syntax->variable;
                if (!v) {
                     printf("Should only get the sizeof(variable) for valid variables\n");
                     assert(0);
                }
                // TODO : get the real size of variable (int, long ...)
                pscope = scope_del(pscope);   
                syntax = immediate_new("4");
                list_push(pscope->parser_stack, syntax);
            }
            else {
                List *l = list_swap(pscope->arguments_stack);
                pscope->arguments_stack = list_new();
                pscope = scope_del(pscope);   
                list_push(pscope->parser_stack, function_call_new((char*)$1, l));
            }
        }
        |
        '(' expression ')'
        {
            // Nothing to do, we have the AST node already.
        }
        ;
