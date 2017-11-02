
%{

/* ----------------------------------------------------------------
 * 
 * Brave Algorithms Build Your Code
 * 
 * ---------------------------------------------------------------- */
 
#define YYSTYPE char*
#include "y.tab.h"

void yyerror();

static void comment(void);
static char *number(char *n, int radix);
static char *symbol(char *);

%}

%%
"#include"    { return T_INCLUDE; }
#[^\n]*       { /* Discard preprocessor comments. */ }
"//"[^\n]*    { /* Discard c99 comments. */ }
"/*"          { comment(); }
[ \t\n]+      { /* Ignore whitespace */ }

"{"           { return T_OPEN_BRACE; }
"}"           { return T_CLOSE_BRACE; }
"("           { return '('; }
")"           { return ')'; }
"~"           { return '~'; }
"!"           { return '!'; }
"+"           { return '+'; }
"-"           { return '-'; }
"*"           { return '*'; }
"/"           { return '/'; }
"%"           { return '%'; }
"|"           { return '|'; }
"&"           { return '&'; }
"^"           { return '^'; }
">>"          { return T_RSHIFT; }
"<<"          { return T_LSHIFT; }
">="          { return T_LARGER_OR_EQUAL; }
"<="          { return T_LESS_OR_EQUAL; }
"=="          { return T_EQUAL; }
"!="          { return T_NEQUAL; }
"<"           { return '<'; }
">"           { return '>'; }
"="           { return '='; }
";"           { return ';'; }
","           { return ','; }
"&"           { return '&'; }
[1-9][0-9]*   {
                yylval = number(yytext, 10); return T_NUMBER;
              }
0x[0-9a-fA-F]+  {
                yylval = number(yytext, 16); return T_NUMBER;
              }
0[0-7]*       {
                yylval = number(yytext, 8); return T_NUMBER;
              }
"if"          { return T_IF; }
"while"       { return T_WHILE; }
"return"      { return T_RETURN; }
"static"      { return T_STATIC; }
"int"         { yylval = symbol(yytext); return T_TYPE; }
[a-zA-Z_][0-9a-zA-Z_]* { yylval = symbol(yytext); return T_IDENTIFIER; }

"<"[a-z.]+">" { return T_HEADER_NAME; }

%%

#define INPUT_EOF 0

static void comment(void) {
    /* Consume characters up to the closing comment marker. */
    char c, prev = 0;
  
    while ((c = input()) != INPUT_EOF) {
        if (c == '/' && prev == '*')
            return;
        prev = c;
    }
    yyerror("unterminated comment");
}

static char **raw_symbol_table = NULL;
static int raw_symbol_size = 0;

static char *symbol(char *s)
{
   for (int i = 0; i < raw_symbol_size; i++)
   { 
      if (!strcmp(s, raw_symbol_table[i]))
      {
         return raw_symbol_table[i];
      }
   }
   raw_symbol_table = realloc(raw_symbol_table, ++raw_symbol_size * sizeof (char *));
   return (raw_symbol_table[raw_symbol_size - 1] = strdup(s));
}

static char *number(char *n, int radix)
{
    char temp[30];
    unsigned long long v = strtoull(n, NULL, radix);
    snprintf(temp, 30, "%llu", v);
    return symbol(temp);
}


