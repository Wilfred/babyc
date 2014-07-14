%{
#include <stdio.h>
#include <string.h>
#include "syntax.c"

void yyerror(const char *str)
{
	fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
	return 1;
}

extern FILE *yyin;

static Syntax *syntax;

void write_skeleton() {
    FILE *out = fopen("out.s", "wb");

    fprintf(out, ".text\n");
    // We seem to require at least 8 spaces for indentation.
    fprintf(out, "    .global _start\n\n");
    fprintf(out, "_start:\n");

    // Exit code as specified.
    // TODO: convert to hex properly.
    fprintf(out, "    movl    $%d, %%ebx\n", syntax->value);

    fprintf(out, "    movl    $1, %%eax\n");
    fprintf(out, "    int     $0x80\n");
    
    fclose(out);
}

int main(int argc, char *argv[])
{
    ++argv, --argc;  /* Skip over program name. */
    if (argc != 1) {
        printf("Please specify a file to compile.\n");
        printf("    $ babyc <your file here>\n");
        return 1;
    }

    yyin = fopen(argv[0], "r");

    if (yyin == NULL) {
        // TODO: work out what the error was.
        // TODO: Unit test this.
        printf("Failed to open file.\n");
        return 2;
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
	function
        ;

function:
	TYPE IDENTIFIER '(' ')' OPEN_BRACE expression CLOSE_BRACE
        ;

expression:
	RETURN NUMBER ';'
        {
            // TODO: fix the memory leak here.
            Syntax *immediate = malloc(sizeof(Syntax));
            immediate->value = $2;
            syntax = immediate;
        }
        ;

