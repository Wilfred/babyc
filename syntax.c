typedef enum { IMMEDIATE } SyntaxType;

typedef struct {
    SyntaxType type;
    union {
        int value;
    };
} Syntax;
