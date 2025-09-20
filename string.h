#pragma once

#include <stdlib.h>

typedef struct {
    size_t len;
    size_t cap;
    char *items;
} String;

String string_from_file(int);
void string_extend_while(String *, const String *, size_t, int(char));
String string_from_cstr(char *);
