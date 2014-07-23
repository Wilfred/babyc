#ifndef BABYC_STACK_HEADER
#define BABYC_STACK_HEADER

typedef struct Stack {
    int size;
    void **content;
} Stack;

Stack *stack_new();

void stack_free(Stack *stack);
void stack_push(Stack *stack, void *item);
void *stack_pop(Stack *stack);

#endif
