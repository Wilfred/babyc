%{
#include <stdio.h>
#include <string.h>

void yyerror(const char *str)
{
	fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
	return 1;
}

extern FILE *yyin;

int main(int argc, char *argv[])
{
    ++argv, --argc;  /* Skip over program name. */
    if (argc > 0) {
        yyin = fopen(argv[0], "r");
    } else {
        yyin = stdin;
    }

    return yyparse();
}

%}

%token INCLUDE HEADER_NAME
%token TYPE IDENTIFIER RETURN NUMBER
%token OPEN_BRACE CLOSE_BRACE

%%

program:
	header function
        ;

header:
        INCLUDE HEADER_NAME
        ;

function:
	TYPE IDENTIFIER '(' ')' OPEN_BRACE expression CLOSE_BRACE
        ;

expression:
	RETURN NUMBER ';'
        ;

