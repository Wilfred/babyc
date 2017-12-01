
/* ----------------------------------------------------------------
 *
 * BabyC Toy compiler for educational purposes
 *
 * ---------------------------------------------------------------- */

#ifndef AST_ANNNOTATE_H_H
#define AST_ANNNOTATE_H_H

#include "syntax.h"

void ast_annotate_syntax_tree(Syntax *syntax);

void ast_integer_set_str(AstInteger *p, char *str, int radic);
void ast_integer_set_bool(AstInteger *p, bool b);
void ast_integer_set_int(AstInteger *p, int i);

char *ast_integer_get_str(AstInteger *p, char *str, int len);
int ast_integer_get_int(AstInteger *p);
unsigned int ast_integer_get_uint(AstInteger *p);
unsigned long long ast_integer_get_unsigned_long_long(AstInteger *p);

void ast_integer_binary_operation(AstInteger *p, AstInteger *a, AstInteger *b,
                                  BinaryExpressionType type);
void ast_integer_unary_operation(AstInteger *p, AstInteger *b,
                                 UnaryExpressionType type);

bool ast_integer_is_zero(AstInteger *p);
bool ast_integer_is_one(AstInteger *p);

#endif
