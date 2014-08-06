#include <stdlib.h>
#include <stdio.h>
#include "list.h"

List *list_new(void) {
    List *list = malloc(sizeof(List));
    list->size = 0;
    list->items = NULL;

    return list;
};

void list_free(List *list) {
    if (list->items != NULL) {
        free(list->items);
    }
    free(list);
}

void list_append(List *list, void *item) {
    list->size++;
    list->items = realloc(list->items, list->size * sizeof(*item));

    list->items[list->size - 1] = item;
}

/* Remove the last item from the list, and return it.
 */
void *list_pop(List *list) {
    void* value = list_get(list, list->size - 1);

    list->size--;
    list->items = realloc(list->items, list->size * sizeof(*value));

    return value;
}

void *list_get(List *list, int index) {
    return list->items[index];
}

void list_set(List *list, int index, void *value) {
    if (index < 0 || index > list->size) {
        printf("WARNING: Index %d is out of bounds!", index);
    }

    else if (index == list->size) {
        list_append(list, value);
    }

    else {
        list->items[index] = value;
    }
}
