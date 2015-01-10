#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
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

int list_length(List *list) { return list->size; }

void list_append(List *list, void *item) {
    list->size++;
    list->items = realloc(list->items, list->size * sizeof(item));

    list->items[list->size - 1] = item;
}

/* Insert item as the first element in list. */
void list_push(List *list, void *item) {
    list->size++;

    void **new_items = malloc(list->size * sizeof(item));
    memcpy(new_items + 1, list->items, (list->size - 1) * sizeof(item));

    if (list->items != NULL) {
        free(list->items);
    }
    list->items = new_items;

    list->items[0] = item;
}

/* Remove the last item from the list, and return it.
 */
void *list_pop(List *list) {
    void *value = list_get(list, list->size - 1);

    list->size--;
    list->items = realloc(list->items, list->size * sizeof(value));

    return value;
}

void *list_get(List *list, int index) { return list->items[index]; }

void list_set(List *list, int index, void *value) {
    if (index < 0 || index > list->size) {
        warnx("Index %d is out of bounds!", index);
    }

    else if (index == list->size) {
        list_append(list, value);
    }

    else {
        list->items[index] = value;
    }
}
