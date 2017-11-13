
/* ----------------------------------------------------------------
 *
 * BabyC toy compiler
 *
 * ---------------------------------------------------------------- */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

/* -----------------------------------------------------------
 * Helper functions
 * A pointer container
 * - using push/pop/peek , it is a stack
 * - using length/index/prepend/append, it is a dynamic array
 * ----------------------------------------------------------- */

List *list_new(void) {
    List *list = malloc(sizeof(List));

    list->size = 0;
    list->items = 0;

    return list;
};

void list_free(List *list) {
    if (list && list->items)
        free(list->items);
    if (list)
        free(list);
}

int list_length(List *list) { return list->size; }

/* Insert item as the last element in list. */
void list_append(List *list, void *item) {
    list->items = realloc(list->items, ++list->size * sizeof(item));
    list->items[list->size - 1] = item;
}

void list_push(List *list, void *item) { list_append(list, item); }

/* Insert item as the first element in list. */
void list_prepend(List *list, void *item) {
    void **new_items = malloc(++list->size * sizeof(item));

    if (list->size > 1) {
        memcpy(new_items + 1, list->items, (list->size - 1) * sizeof(item));
    }
    free(list->items);
    new_items[0] = item;
    list->items = new_items;
}

/* Remove the last item from the list, and return it.
 */
void *list_last(List *list) {
    assert(list->size > 0);

    void *item = list_get(list, list->size - 1);

    list->items = realloc(list->items, --list->size * sizeof(item));
    return item;
}

void *list_pop(List *list) { return list_last(list); }

void *list_peek(List *list) { return list_get(list, list->size - 1); }

void *list_get(List *list, int index) {
    assert(index < list->size);
    return list->items[index];
}

void list_set(List *list, int index, void *item) {
    if (index < 0 || index > list->size) {
        printf("Index %d is out of bounds!", index);
        assert(0);
    } else if (index == list->size) {
        list_append(list, item);
    } else {
        list->items[index] = item;
    }
}

List *list_swap(List *list) {
    for (int i = 0, j = list->size - 1; i < list->size / 2; i++, j--) {
        void *t = list->items[i];

        list->items[i] = list->items[j];
        list->items[j] = t;
    }
    return list;
}
