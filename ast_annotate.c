
/* ----------------------------------------------------------------
 *
 * BabyC Toy compiler for educational purposes
 *
 * ---------------------------------------------------------------- */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "syntax.h"
#include "log_error.h"
#include "list.h"
#include <x86intrin.h>

bool ast_optimize = false;

/* ----------------------------------------------------------------
 * Get the right type on intermediate AST values
 * ---------------------------------------------------------------- */

ObjectType ast_annotate_expressions_syntax(Syntax *syntax, ObjectType dest);
ObjectType ast_annotate_expressions_list(List *list, ObjectType dest);
ObjectType ast_convert_value_to_type(char *hexvalue) {
    char *pt = hexvalue + strlen(hexvalue);
    ObjectType type = O_UINT8;

    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    type = O_UINT16;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    type = O_UINT32;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    type = O_UINT64;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    if (--pt == hexvalue)
        return type;
    type = O_UINT128;
    return type;
}

ObjectType ast_annotate_expressions_syntax(Syntax *syntax,
                                           ObjectType destType) {
    ObjectType type;

    if (syntax->type == BLOCK) {
        type =
            ast_annotate_expressions_list(syntax->block->statements, O_UNDEF);
    } else if (syntax->type == UNARY_OPERATOR) {
        type = ast_annotate_expressions_syntax(
            syntax->unary_expression->expression, destType);
        if (syntax->unary_expression->objectType == O_UNDEF) {
            syntax->unary_expression->objectType = type;
        }
    } else if (syntax->type == BINARY_OPERATOR) {
        ObjectType r = ast_annotate_expressions_syntax(
            syntax->binary_expression->right, destType);
        ObjectType l = ast_annotate_expressions_syntax(
            syntax->binary_expression->left, destType);

        type = (r > l) ? r : l;
        if (syntax->binary_expression->objectType == O_UNDEF) {
            syntax->binary_expression->objectType = type;
        }
    } else if (syntax->type == FUNCTION_ARGUMENT) {
        type = syntax->function_argument->objectType;
        ast_annotate_expressions_syntax(syntax->function_argument->expression,
                                        type);
    } else if (syntax->type == FUNCTION_PARAMETER) {
        type = syntax->function_parameter->variable->objectType;
    } else if (syntax->type == FUNCTION_CALL) {
        ast_annotate_expressions_list(syntax->function_call->arguments,
                                      O_UNDEF);
        type = syntax->function_call->objectType;
    } else if (syntax->type == FUNCTION_DEFINITION) {
        ast_annotate_expressions_syntax(syntax->function_definition->block,
                                        O_UNDEF);
        type = syntax->function_definition->objectType;
    } else if (syntax->type == WHILE_STATEMENT) {
        ast_annotate_expressions_syntax(syntax->while_statement->body, O_UNDEF);
        ast_annotate_expressions_syntax(syntax->while_statement->condition,
                                        O_BOOL);
        type = O_UNDEF;
    } else if (syntax->type == IF_STATEMENT) {
        ast_annotate_expressions_syntax(syntax->if_statement->condition,
                                        O_BOOL);
        ObjectType r = ast_annotate_expressions_syntax(
            syntax->if_statement->if_then, O_UNDEF);
        ObjectType l = ast_annotate_expressions_syntax(
            syntax->if_statement->if_else, O_UNDEF);

        type = (r > l) ? r : l;
    } else if (syntax->type == GOTO_STATEMENT) {
        type = O_UNDEF;
    } else if (syntax->type == BREAK_STATEMENT) {
        type = O_UNDEF;
    } else if (syntax->type == CONTINUE_STATEMENT) {
        type = O_UNDEF;
    } else if (syntax->type == LABEL_STATEMENT) {
        type = O_UNDEF;
    } else if (syntax->type == NOP_STATEMENT) {
        type = O_UNDEF;
    } else if (syntax->type == ASSIGNMENT_STATEMENT) {
        type = syntax->assignment->variable->objectType;
        ast_annotate_expressions_syntax(syntax->assignment->expression, type);
    } else if (syntax->type == RETURN_STATEMENT) {
        type = syntax->return_statement->objectType;
        ast_annotate_expressions_syntax(syntax->return_statement->expression, type);
    } else if (syntax->type == ASSIGNMENT_STATIC) {
        type = syntax->assignment->variable->objectType;
        ast_annotate_expressions_syntax(syntax->assignment->expression, type);
    } else if (syntax->type == VARIABLE) {
        type = syntax->variable->objectType;
    } else if (syntax->type == ADDRESS) {
        ast_annotate_expressions_syntax(syntax->address->offset, O_INT32);
        type = ast_annotate_expressions_syntax(syntax->address->identifier,
                                               destType);
    } else if (syntax->type == READ_ADDRESS) {
        type = ast_annotate_expressions_syntax(syntax->read_address->address,
                                               destType);
    } else if (syntax->type == WRITE_ADDRESS) {
        type = ast_annotate_expressions_syntax(syntax->write_address->address,
                                               O_UNDEF);
        ast_annotate_expressions_syntax(syntax->write_address->offset, O_INT32);
        type = ast_annotate_expressions_syntax(
            syntax->write_address->expression, type);
    } else if (syntax->type == IMMEDIATE) {
        type = syntax->immediate->value.objectType;
    } else {
        type = O_UNDEF;
    }
    return type;
}

ObjectType ast_annotate_expressions_list(List *list, ObjectType destType) {

    for (int i = 0; i < list_length(list); i++) {
        ast_annotate_expressions_syntax(list_get(list, i), destType);
    }
    return O_UNDEF;
}

/* ----------------------------------------------------------------
 * Resolve function calls and function parameters
 * ---------------------------------------------------------------- */
void ast_annotate_function_calls_syntax(Syntax *syntax, Syntax *root);
void ast_annotate_function_calls_list(List *list, Syntax *root);

void ast_link_function_definition(FunctionCall *function_call, Syntax *root) {
    if (root->type != BLOCK) {
        log_error("Unvalid AST root");
    }
    char *name = function_call->name;
    int alen = list_length(function_call->arguments);
    List *top = root->block->statements;
    int jlen = list_length(top);

    for (int j = 0; j < jlen; j++) {
        Syntax *s = list_get(top, j);

        if (s->type == FUNCTION_DEFINITION) {
            FunctionDefinition *function_definition = s->function_definition;

            if (!strcmp(name, function_definition->name)) {
                int plen = list_length(function_definition->parameters);

                if (plen != alen) {
                    log_error("Parameter mismatch on function call '%s'", name);
                }
                function_call->objectType = function_definition->objectType;
                for (int i = 0; i < alen; i++) {
                    Syntax *sp = list_get(function_definition->parameters, i);
                    Syntax *sa = list_get(function_call->arguments, i);
                    FunctionParameter *p = sp->function_parameter;
                    FunctionArgument *a = sa->function_argument;

                    a->objectType = p->variable->objectType;
                }
                return;
            }
        }
    }
}

void ast_annotate_function_calls_syntax(Syntax *syntax, Syntax *root) {
    if (syntax->type == BLOCK) {
        ast_annotate_function_calls_list(syntax->block->statements, root);
    } else if (syntax->type == UNARY_OPERATOR) {
        ast_annotate_function_calls_syntax(syntax->unary_expression->expression,
                                           root);
    } else if (syntax->type == BINARY_OPERATOR) {
        ast_annotate_function_calls_syntax(syntax->binary_expression->right,
                                           root);
        ast_annotate_function_calls_syntax(syntax->binary_expression->left,
                                           root);
    } else if (syntax->type == FUNCTION_ARGUMENT) {
        // todo : get type from function parameters
        ast_annotate_function_calls_syntax(
            syntax->function_argument->expression, root);
    } else if (syntax->type == FUNCTION_PARAMETER) {
    } else if (syntax->type == FUNCTION_CALL) {
        ast_link_function_definition(syntax->function_call, root);
        ast_annotate_function_calls_list(syntax->function_call->arguments,
                                         root);
    } else if (syntax->type == FUNCTION_DEFINITION) {
        ast_annotate_function_calls_syntax(syntax->function_definition->block,
                                           root);
    } else if (syntax->type == WHILE_STATEMENT) {
        ast_annotate_function_calls_syntax(syntax->while_statement->body, root);
        ast_annotate_function_calls_syntax(syntax->while_statement->condition,
                                           root);
    } else if (syntax->type == IF_STATEMENT) {
        ast_annotate_function_calls_syntax(syntax->if_statement->condition,
                                           root);
        ast_annotate_function_calls_syntax(syntax->if_statement->if_then, root);
        ast_annotate_function_calls_syntax(syntax->if_statement->if_else, root);
    } else if (syntax->type == GOTO_STATEMENT) {
    } else if (syntax->type == BREAK_STATEMENT) {
    } else if (syntax->type == CONTINUE_STATEMENT) {
    } else if (syntax->type == LABEL_STATEMENT) {
    } else if (syntax->type == NOP_STATEMENT) {
    } else if (syntax->type == ASSIGNMENT_STATEMENT) {
        ast_annotate_function_calls_syntax(syntax->assignment->expression,
                                           root);
    } else if (syntax->type == RETURN_STATEMENT) {
        ast_annotate_function_calls_syntax(syntax->return_statement->expression,
                                           root);
    } else if (syntax->type == ASSIGNMENT_STATIC) {
        ast_annotate_function_calls_syntax(syntax->assignment->expression,
                                           root);
    } else if (syntax->type == VARIABLE) {
    } else if (syntax->type == ADDRESS) {
        ast_annotate_function_calls_syntax(syntax->address->offset, root);
        ast_annotate_function_calls_syntax(syntax->address->identifier, root);
    } else if (syntax->type == READ_ADDRESS) {
        ast_annotate_function_calls_syntax(syntax->read_address->address, root);
    } else if (syntax->type == WRITE_ADDRESS) {
        ast_annotate_function_calls_syntax(syntax->write_address->address,
                                           root);
        ast_annotate_function_calls_syntax(syntax->write_address->offset, root);
        ast_annotate_function_calls_syntax(syntax->write_address->expression,
                                           root);
    } else if (syntax->type == IMMEDIATE) {
    }
}

void ast_annotate_function_calls_list(List *list, Syntax *root) {
    for (int i = 0; i < list_length(list); i++) {
        ast_annotate_function_calls_syntax(list_get(list, i), root);
    }
}

void ast_annotate_syntax_tree(Syntax *syntax) {
    ast_annotate_function_calls_syntax(syntax, syntax);

    ast_annotate_expressions_syntax(syntax, O_UNDEF);
}

/* ----------------------------------------------------------------
 * Integer immediate in AST tree
 * ---------------------------------------------------------------- */

void ast_set_object_type(AstInteger *p) {
    if (p->val[1] &  (1ull << 63)) {
        p->objectType = O_INT128;
    }
    else if (p->val[1]) {
        p->objectType = O_UINT128;
    } else if (p->val[0] & (1ull << 63)) {
        p->objectType = O_UINT128;
    } else if (p->val[0] & 0xffffffff00000000ull) {
        p->objectType = O_UINT64;
    } else if (p->val[0] & (1ull << 31)) {
        p->objectType = O_UINT64;
    } else if (p->val[0] & 0xffff0000ull) {
        p->objectType = O_UINT32;
    } else if (p->val[0] & (1ull << 15)) {
        p->objectType = O_UINT32;
    } else if (p->val[0] & 0xff00ull) {
        p->objectType = O_UINT16;
    } else if (p->val[0] & (1ull << 7)) {
        p->objectType = O_UINT16;
    } else {
        p->objectType = O_UINT8;
    }
}

void ast_integer_set_str(AstInteger *p, char *str, int radix) {
    char c, *s = str;
    uint64_t mask = (1ull << 56) - 1;
    uint64_t val0 = 0, val1 = 0, val2 = 0;

    while ((c = *s++) != 0) {
        val0 *= radix;
        val1 *= radix;
        val2 *= radix;
        
        if (c >= 'a')
            val0 += c - 'a' + 10;
        else if (c >= 'A')
            val0 += c - 'A' + 10;
        else if (c >= '0')
            val0 += c - '0';
            
        if (val0 > mask) {
            val1 += val0 >> 56;
            val0 &= mask;
        }
        if (val1 > mask) {
            val2 += val1 >> 56;
            val1 &= mask;
        }
        if (val2 > mask) {
            log_error("integer overflow : '%s' too large", str);
        }
    }
    p->val[0] = val0 | (val1 << 56);
    p->val[1] = (val1 >> 8) | (val2 << 48);
    if (val2 >> 16) {
        log_error("integer overflow : '%s' too large", str);
    }
    ast_set_object_type(p);
}

static uint64_t mul_u64(uint64_t a, uint64_t b, uint64_t *hi) {
    uint32_t a_lo = a;
    uint32_t a_hi = a >> 32;
    uint32_t b_lo = b;
    uint32_t b_hi = b >> 32;
    uint64_t lo = (uint64_t)a_lo * b_lo;
    uint64_t t2 = (uint64_t)a_lo * b_hi;
    uint64_t t3 = (uint64_t)a_hi * b_lo;
    uint64_t t4 = (uint64_t)a_hi * b_hi;

    t4 += _addcarry_u64(0, lo, t2 << 32, (unsigned long long *)&lo);
    t4 += _addcarry_u64(0, lo, t3 << 32, (unsigned long long *)&lo);
    t4 += t2 >> 32;
    t4 += t3 >> 32;
    *hi = t4;
    return lo;
}

static uint64_t div10_u64(uint64_t hi, uint64_t lo, uint64_t *mod)
{
    uint64_t t, d, r;
    
    t = hi;
    mul_u64(t, 0xcccccccccccccccdull, &d); 
    d >>= 3;
    r = t - d * 10;
    t = (r << 32) + (lo >> 32);
    mul_u64(t, 0xcccccccccccccccdull, &d); 
    d >>= 3;
    r =  t- d * 10;
    t = (r << 32) + (lo & 0xffffffff);
    mul_u64(t, 0xcccccccccccccccdull, &d); 
    d >>= 3;
    *mod = t - d * 10;
    return d;
}

char *ast_integer_get_str(AstInteger *p, char *str, int len) {
    uint64_t val0 = p->val[0];
    uint64_t val1 = p->val[1];
    char *s = str + len - 1;
    *--s = 0;

    do
    {
        uint64_t mod = 0;
        val1 = div10_u64(mod, val1, &mod);
        val0 = div10_u64(mod, val0, &mod);
        *--s = mod + '0';
    }
    while (val1 | val0);
    return s;
}

void ast_integer_set_int(AstInteger *p, int i) {
    if (i < 0) {
        char b = _subborrow_u64(0, i, 0, (unsigned long long *)&p->val[0]);
        b = _subborrow_u64(b, 0, 0, (unsigned long long *)&p->val[1]);
    } else {
        p->val[0] = i;
        p->val[1] = 0;
    }
    ast_set_object_type(p);
}

void ast_integer_set_bool(AstInteger *p, bool b) {
    p->val[0] = b ? 1 : 0;
    p->val[1] = 0;
    p->objectType = O_UINT8; // TODO O_BOOL ?
}

bool ast_integer_is_zero(AstInteger *p) {
    return (p->val[0] == 0 && p->val[1] == 0);
}

bool ast_integer_is_one(AstInteger *p) {
    return (p->val[0] == 1 && p->val[1] == 0);
}

int ast_integer_get_int(AstInteger *p, int bits) {
    int ret = 0;
    if (bits <= 32) {
        ret = (p->val[0] & 0xffffffffull);
    }
    if (p->val[1] == (uint64_t)-1 && (p->val[0] >> 31) == 0x1ffffffffull)
    {
        /* negative number is ok */
    }
    else if (p->val[1] || p->val[0] > 0xffffffffull) {
        log_error("signed integer loss of accuracy when getting %d bits", bits);
    }
    return ret;
}

unsigned ast_integer_get_uint(AstInteger *p, int bits) {
    unsigned ret = 0;
    if (bits <= 32) {
        ret = (p->val[0] & 0xffffffff);
    }
    return ret;
}

unsigned long long ast_integer_get_unsigned_long_long(AstInteger *p, int bits) {
    unsigned long long ret = 0;
    if (bits <= 64) {
        ret = p->val[0];
    }
    if (p->val[1]) {
        log_error("unsigned integer loss of accuracy when getting %d bits", bits);
    }
    return ret;
}

void ast_integer_binary_operation(AstInteger *res, AstInteger *left,
                                  AstInteger *right,
                                  BinaryExpressionType binary_type) {
    if (binary_type == MULTIPLICATION) {
        if (left->val[1] | right->val[1]) {
            // TODO
            assert(0);
        }
        res->val[0] = mul_u64(left->val[0], right->val[0], &res->val[1]);
        res->val[1] = 0;
    } else if (binary_type == DIVISION) {
        if (left->val[1] | right->val[1]) {
            // TODO
            assert(0);
        }
        if (right->val[0] == 0)
            log_error("Divide by zero !\n");
        res->val[0] = left->val[0] / right->val[0];
        res->val[1] = 0;
    } else if (binary_type == MODULUS) {
        if (left->val[1] | right->val[1]) {
            // TODO
            assert(0);
        }
        if (right->val[0] == 0)
            log_error("Modulus is zero !\n");
        res->val[0] = left->val[0] % right->val[0];
        res->val[1] = 0;
    } else if (binary_type == OR) {
        res->val[0] = left->val[0] | right->val[0];
        res->val[1] = left->val[1] | right->val[1];
    } else if (binary_type == AND) {
        res->val[0] = left->val[0] & right->val[0];
        res->val[1] = left->val[1] & right->val[1];
    } else if (binary_type == XOR) {
        res->val[0] = left->val[0] ^ right->val[0];
        res->val[1] = left->val[1] ^ right->val[1];
    } else if (binary_type == RSHIFT) {
        int s  = right->val[0];
        if (right->val[1] || s >= 128) {
            res->val[0] = 0;
            res->val[1] = 0;
        } else if (s >= 64) {
            s -= 64;
            res->val[0] = left->val[1] >> s;
            res->val[1] = 0;
        } else {
            res->val[0] = (left->val[0] >> s) | (res->val[1] << (64 - s));
            res->val[1] = (left->val[1] >> s);
        }
    } else if (binary_type == LSHIFT) {
        int s  = right->val[0];
        if (right->val[1] || s >= 128) {
            res->val[0] = 0;
            res->val[1] = 0;
        } else if (right->val[0] >= 64) {
            s -= 64;
            res->val[1] = left->val[0] << s;
            res->val[0] = 0;
        } else {
            res->val[1] = (left->val[1] << s) | (left->val[0] >> (64 - s));
            res->val[0] = (left->val[0] << s);
        }
    } else if (binary_type == ADDITION) {
            char c = _addcarry_u64(0, left->val[0], right->val[0],
                                   (unsigned long long *)&res->val[0]);

            _addcarry_u64(c, left->val[1], right->val[1],
                              (unsigned long long *)&res->val[1]);
    } else if (binary_type == SUBTRACTION) {
            char b = _subborrow_u64(0, right->val[0], left->val[0],
                                    (unsigned long long *)&res->val[0]);

            _subborrow_u64(b, right->val[1], left->val[1],
                               (unsigned long long *)&res->val[1]);
    } else if (binary_type == LESS_THAN || binary_type == LARGER_THAN ||
               binary_type == LESS_THAN_OR_EQUAL ||
               binary_type == LARGER_THAN_OR_EQUAL || binary_type == EQUAL ||
               binary_type == NEQUAL) {
        unsigned long long t0, t1;
        char b = _subborrow_u64(0, right->val[0], left->val[0], &t0);
            b = _subborrow_u64(b, right->val[1], left->val[1], &t1);

        bool c = (b != 0);
        bool z = ((t0 | t1) == 0);
        bool t;

        switch (binary_type) {
        case LESS_THAN:
            t = c;
            break;
        case LARGER_THAN:
            t = !c && !z;
            break;
        case LESS_THAN_OR_EQUAL:
            t = c || z;
            break;
        case LARGER_THAN_OR_EQUAL:
            t = !c;
            break;
        case EQUAL:
            t = !c && z;
            break;
        case NEQUAL:
            t = c || !z;
            break;
        default:
            t = false;
        }
        res->val[0] = t;
        res->val[1] = 0;
    } else {
        log_error("Invalid binary type %d\n", binary_type);
    }
    ast_set_object_type(res);
}

void ast_integer_unary_operation(AstInteger *res, AstInteger *b,
                                 UnaryExpressionType unary_type) {
    if (unary_type == BITWISE_NEGATION) {
        res->val[0] = ~b->val[0];
        res->val[1] = ~b->val[1];
    } else if (unary_type == ARITHMETIC_NEGATION) {
        char c = _subborrow_u64(0, b->val[0], 0, (unsigned long long *)&res->val[0]);
        _subborrow_u64(c, b->val[1], 0, (unsigned long long *)&res->val[1]);
    } else if (unary_type == LOGICAL_NEGATION) {
        res->val[0] = ast_integer_is_zero(b);
        res->val[1] = 0;
    } else {
        log_error("Invalid unary type %d\n", unary_type);
    }
    ast_set_object_type(res);
}
