
/* ----------------------------------------------------------------
 *
 * Brave Algorithms Build Your Code
 *
 * ---------------------------------------------------------------- */

#ifndef BABYC_LIST_HEADER
#define BABYC_LIST_HEADER

/* -----------------------------------------------------------
 * Helper functions
 * A pointer container
 * - using push/pop/peek , it is a stack
 * - using length/index/prepend/append/get/set, it is a dynamic array
 * ----------------------------------------------------------- */

typedef struct List {
    int size;
    void **items;
} List;

List *list_new(void);

int list_length(List *list);

void list_free(List *list);

void list_append(List *list, void *item);
void list_push(List *list, void *item);

void list_prepend(List *list, void *item);

void *list_last(List *list);
void *list_pop(List *list);
void *list_peek(List *list);

void *list_get(List *list, int index);

void list_set(List *list, int index, void *value);

List *list_swap(List *list);

#endif
