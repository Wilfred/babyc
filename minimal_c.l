%{
#include <stdio.h>
#include "y.tab.h"
%}


%%
"//"[^\n]*    { /* Discard comments. */ }
[ \t\n]+      { /* Ignore whitespace */ }

"{"           { return OPEN_BRACE; }
"}"           { return CLOSE_BRACE; }
"("           { return '('; }
")"           { return ')'; }
";"           { return ';'; }
[0-9]+        { return NUMBER; }
"return"      { return RETURN; }

"int"         { return TYPE; }
"main"        { return IDENTIFIER; }

"#include"    { return INCLUDE; }
"<"[a-z.]+">" { return HEADER_NAME; }
%%
