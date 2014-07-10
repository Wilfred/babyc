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

void write_skeleton() {
    FILE *out = fopen("out.s", "wb");

    fprintf(out, ".text\n");
    // We seem to require at least 8 spaces for indentation.
    fprintf(out, "    .global _start\n\n");
    fprintf(out, "_start:\n");

    // Exit code of zero.
    fprintf(out, "    movl    $0, %%ebx\n");

    fprintf(out, "    movl    $1, %%eax\n");
    fprintf(out, "    int     $0x80\n");
    
    fclose(out);
}

int main(int argc, char *argv[])
{
    ++argv, --argc;  /* Skip over program name. */
    if (argc > 0) {
        yyin = fopen(argv[0], "r");
    } else {
        yyin = stdin;
    }

    yyparse();

    write_skeleton();
    printf("Written out.s.\n");
    printf("Build it with:\n");
    printf("    $ as out.s -o out.o\n");
    printf("    $ ld -s -o out out.o\n");

    return 0;
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

