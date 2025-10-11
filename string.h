#pragma once

#include <stdlib.h>

typedef struct {
    size_t len;
    size_t cap;
    char *items;
} String;

typedef struct {
    String array;
    size_t position;
} CharIter;

String string_from_file(int);
String string_from_cstr(char *);
int stringcmp(const String *s, const String *t);
