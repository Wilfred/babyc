
%option prefix="bb"

%{

/* ----------------------------------------------------------------
 * 
 * BabyC Toy compiler for educational purposes
 * 
 * ---------------------------------------------------------------- */

#include "bb_type.h"
#include "bb.tab.h"
#include "log_error.h"

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

static char *char_escape(char *n, int radix)
{
    char temp[30];
    if (radix == 8)
    {
        unsigned long long v = strtoull(n+2, NULL, 8);
        snprintf(temp, 30, "%llu", v);
    }
    else if (radix == 16)
    {
        unsigned long long v = strtoull(n+3, NULL, 16);
        snprintf(temp, 30, "%llu", v);
    }
    else
    {
        unsigned char u = n[1];
        snprintf(temp, 30, "%u", (unsigned)u);
    }
    return symbol(temp);    
}

static int line_number = 1;

#define LVAL(s) do { bblval.line = line_number; if (s) bblval.symbol = s; } while (0)

%}

%option nounput
%option noinput
%option noyywrap

%x comment
%x include

IDENT [a-zA-Z_][0-9a-zA-Z_]*

%%

^"#include"  { BEGIN(include); }
<include>[ \t]+ { }
<include>"<"[0-9_a-z.]+ { }
<include>">" { BEGIN(0); }

^"#"[^\n]*                 { /* Discard preprocessor comments. */ }

\"(\\.|[^"\\])*\"       { LVAL(0); bblval.symbol = symbol(yytext); return T_STRING; }

"//"[^\n]*              { /* Discard comment */ }

"/*"                    { BEGIN(comment); }

<comment>[^*\n]*        { /* eat comment */ }
<comment>"*"?\n         { ++line_number; /* eat comment */ }
<comment>"*"[^/\n]      { /* eat comment */ }
<comment>"*/"           { BEGIN(INITIAL); }

[ \t\r]+        { /* Ignore whitespace */ }
\n            { ++line_number; }


"++"          { LVAL(0); return T_INCREMENT; }
"--"          { LVAL(0); return T_DECREMENT; }
"||"          { LVAL(0); return T_LOGICAL_OR; }
"&&"          { LVAL(0); return T_LOGICAL_AND; }
">>"          { LVAL(0); return T_RSHIFT; }
"<<"          { LVAL(0); return T_LSHIFT; }
">="          { LVAL(0); return T_LARGER_OR_EQUAL; }
"<="          { LVAL(0); return T_LESS_OR_EQUAL; }
"=="          { LVAL(0); return T_EQUAL; }
"!="          { LVAL(0); return T_NEQUAL; }

"{"           { LVAL(0); return T_OPEN_BRACE; }
"}"           { LVAL(0); return T_CLOSE_BRACE; }
"("           { LVAL(0); return '('; }
")"           { LVAL(0); return ')'; }
"["           { LVAL(0); return '['; }
"]"           { LVAL(0); return ']'; }
"~"           { LVAL(0); return '~'; }
"!"           { LVAL(0); return '!'; }
"+"           { LVAL(0); return '+'; }
"-"           { LVAL(0); return '-'; }
"*"           { LVAL(0); return '*'; }
"/"           { LVAL(0); return '/'; }
"%"           { LVAL(0); return '%'; }
"|"           { LVAL(0); return '|'; }
"&"           { LVAL(0); return '&'; }
"^"           { LVAL(0); return '^'; }
"<"           { LVAL(0); return '<'; }
">"           { LVAL(0); return '>'; }
"="           { LVAL(0); return '='; }
";"           { LVAL(0); return ';'; }
","           { LVAL(0); return ','; }
":"           { LVAL(0); return ':'; }

[1-9][0-9]*   {
                LVAL(0); bblval.symbol = number(yytext, 10); return T_NUMBER;
              }

0[xX][0-9a-fA-F]+  {
                LVAL(0); bblval.symbol = number(yytext, 16); return T_NUMBER;
              }

0[0-7]*       {
                LVAL(0); bblval.symbol = number(yytext, 8); return T_NUMBER;
              }

"'\\x"[0-9a-zA-Z]+"'" {
                LVAL(0); bblval.symbol = char_escape(yytext, 16); return T_NUMBER;
              }
"'\\"[0-7]+"'" {
                LVAL(0); bblval.symbol = char_escape(yytext, 8); return T_NUMBER;
              }
"'"[\x20-\x7f]"'" {
                LVAL(0); bblval.symbol = char_escape(yytext, 0); return T_NUMBER;
              }
"'\\n'"       {
                LVAL(0); bblval.symbol = symbol("12"); return T_NUMBER;
              }
"'\\r'"       {
                LVAL(0); bblval.symbol = symbol("13"); return T_NUMBER;
              }
"'\\t'"       {
                LVAL(0); bblval.symbol = symbol("8"); return T_NUMBER;
              }

"if"          { LVAL(0); return T_IF; }
"else"        { LVAL(0); return T_ELSE; }
"goto"        { LVAL(0); return T_GOTO; }
"while"       { LVAL(0); return T_WHILE; }
"break"       { LVAL(0); return T_BREAK; }
"continue"    { LVAL(0); return T_CONTINUE; }
"sizeof"      { LVAL(0); return T_SIZEOF; }
"return"      { LVAL(0); return T_RETURN; }
"static"      { LVAL(0); return T_STATIC; }
"void"        { LVAL(0); return T_VOID; }
"unsigned"    { LVAL(0); return T_UNSIGNED; }
"signed"      { LVAL(0); return T_SIGNED; }
"int"|"intptr_t"|"long"|"bool" { LVAL(0); bblval.symbol = symbol(yytext); return T_TYPE; }
[u]?"int"[8163264]+"_t" { LVAL(0); bblval.symbol = symbol(yytext); return T_TYPE; }
{IDENT}/:     { LVAL(0); bblval.symbol = symbol(yytext); return T_LABEL; }
{IDENT}       { LVAL(0); bblval.symbol = symbol(yytext); return T_IDENTIFIER; }

.             { unsigned char c = *yytext;  log_error("unknown byte 0x%02x line %d", (unsigned)c, line_number); }

%%

