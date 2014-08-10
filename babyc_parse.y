%{
#include <stdio.h>
#include <stdbool.h>
#include "syntax.h"
#include "assembly.h"
#include "stack.h"

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

extern FILE *yyin;

Stack *syntax_stack;

int main(int argc, char *argv[])
{
    ++argv, --argc;  /* Skip over program name. */

    bool dump_ast = false;
    if (argc == 1) {
        yyin = fopen(argv[0], "r");
    } else if (argc == 2 && strcmp(argv[0], "--dump-ast") == 0) {
        dump_ast = true;
        yyin = fopen(argv[1], "r");
    } else {
        printf("Usage:\n");
        printf("    $ babyc foo.c\n");
        printf("    $ babyc --dump-ast foo.c\n");
        return 1;
    }

    int result;

    if (yyin == NULL) {
        // TODO: work out what the error was.
        // TODO: Unit test this.
        printf("Failed to open file.\n");
        result = 2;
        goto cleanup_file;
    }

    syntax_stack = stack_new();

    result = yyparse();
    if (result != 0) {
        printf("\n");
        goto cleanup_syntax;
    }

    Syntax *complete_syntax = stack_pop(syntax_stack);
    if (syntax_stack->size > 0) {
        printf("WARNING: Did not consume the whole syntax stack during parsing!");
    }

    if (dump_ast) {
        print_syntax(complete_syntax);
    } else {
        write_assembly(complete_syntax);
        syntax_free(complete_syntax);

        printf("Written out.s.\n");
        printf("Build it with:\n");
        printf("    $ as out.s -o out.o\n");
        printf("    $ ld -s -o out out.o\n");
    }

cleanup_syntax:
    /* TODO: if we exit early from syntactically invalid code, we will
       need to free multiple Syntax structs on this stack.
     */
    stack_free(syntax_stack);
cleanup_file:
    fclose(yyin);

    return result;
}

%}

%token INCLUDE HEADER_NAME
%token TYPE IDENTIFIER RETURN NUMBER
%token OPEN_BRACE CLOSE_BRACE

/* Left associate operators, least precedence first. */
%left '+' '*'

%%

program:
	function
        ;

function:
	TYPE IDENTIFIER '(' ')' OPEN_BRACE statement CLOSE_BRACE
        ;

statement:
        RETURN expression ';'
        {
            Syntax *current_syntax = stack_pop(syntax_stack);
            stack_push(syntax_stack, return_statement_new(current_syntax));
        }
        ;

expression:
	NUMBER
        {
            stack_push(syntax_stack, immediate_new($1));
        }
        |
        '~' expression
        {
            Syntax *current_syntax = stack_pop(syntax_stack);
            stack_push(syntax_stack, bitwise_negation_new(current_syntax));
        }
        |
        '!' expression
        {
            Syntax *current_syntax = stack_pop(syntax_stack);
            stack_push(syntax_stack, logical_negation_new(current_syntax));
        }
        |
        expression '+' expression
        {
            Syntax *left = stack_pop(syntax_stack);
            Syntax *right = stack_pop(syntax_stack);
            stack_push(syntax_stack, addition_new(left, right));
        }
        |
        expression '*' expression
        {
            Syntax *left = stack_pop(syntax_stack);
            Syntax *right = stack_pop(syntax_stack);
            stack_push(syntax_stack, multiplication_new(left, right));
        }
        ;
