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

main()
{
	yyparse();
}

%}

%token TYPE IDENTIFIER RETURN NUMBER
%token OPEN_BRACE CLOSE_BRACE

%%

program:
	function
        ;

function:
	TYPE IDENTIFIER '(' ')' OPEN_BRACE expression CLOSE_BRACE
        ;

expression:
	RETURN NUMBER ';'
        ;

