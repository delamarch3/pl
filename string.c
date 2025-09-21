#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "array.h"
#include "string.h"

String string_from_file(int fd) {
    char *buf = malloc(ARRAY_CAP);
    char *ptr = buf;

    size_t cap = ARRAY_CAP;
    size_t len = 0;
    ssize_t n;
    while ((n = read(fd, ptr, ARRAY_CAP)) > 0) {
        len += n;
        if (len == cap) {
            cap *= ARRAY_MUL;
            buf = realloc(buf, cap);
        }

        ptr = buf;
        ptr += len;
    }

    if (n == -1) {
        fprintf(stderr, "read: %s", strerror(errno));
        exit(1);
    }

    String s = {
        .items = buf,
        .cap = cap,
        .len = len,
    };

    return s;
}

String string_from_cstr(char *s) {
    String t = {0};

    char c;
    while ((c = *s++)) {
        append(&t, c);
    }

    return t;
}
