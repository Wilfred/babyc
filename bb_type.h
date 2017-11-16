
/* ----------------------------------------------------------------
 *
 * BabyC Toy compiler for educational purposes
 *
 * ---------------------------------------------------------------- */

#ifndef BABYC_LEX_TYPE_HEADER
#define BABYC_LEX_TYPE_HEADER

typedef struct {
    char *symbol;
    int line;
} BBSTYPE;

#define YYSTYPE BBSTYPE

#endif
