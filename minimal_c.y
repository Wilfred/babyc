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

%%

program:
	| expression
        ;

function:
	TYPE IDENTIFIER '{' expression '}'
        ;

expression:
	RETURN NUMBER ';'
        ;

