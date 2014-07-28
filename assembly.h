#include <stdio.h>
#include "syntax.h"

#ifndef BABYC_ASSEMBLY_HEADER
#define BABYC_ASSEMBLY_HEADER

void emit_header(FILE *out, char *name);

void emit_insn(FILE *out, char *insn);

void write_header(FILE *out);

void write_footer(FILE *out);

void write_syntax(FILE *out, Syntax *syntax);

void write_assembly(Syntax *syntax);

#endif
