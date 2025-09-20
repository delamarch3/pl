#pragma once

#define ARRAY_CAP 128
#define ARRAY_MUL 2

#define append(array, item)                                                                        \
    if ((array)->cap == 0) {                                                                       \
        (array)->cap = ARRAY_CAP;                                                                  \
    }                                                                                              \
    if ((array)->len == (array)->cap) {                                                            \
        (array)->cap *= ARRAY_MUL;                                                                 \
    }                                                                                              \
    (array)->items = realloc((array)->items, (array)->cap * sizeof((array)->items[0]));            \
    (array)->items[(array)->len++] = item;
