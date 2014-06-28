%{
#include <stdio.h>
#include "y.tab.h"
%}


%%
"//"[^\n]* { /* Discard comments. */ }
[ \t]+     { /* Ignore whitespace */ }

"{"        { return OPEN_BRACE; }
"}"        { return CLOSE_BRACE; }
";"        { return ';'; }
[0-9]+     { return NUMBER; }
"return"   { return RETURN; }

"int"      { return TYPE; }
"main"     { return IDENTIFIER; }
%%
