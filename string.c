#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "array.h"
#include "string.h"

String string_from_file(int fd) {
    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1) {
        fprintf(stderr, "lseek: %s", strerror(errno));
        exit(1);
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "lseek: %s", strerror(errno));
        exit(1);
    }

    char *buf = malloc(size);
    char *ptr = buf;

    ssize_t n;
    while ((n = read(fd, ptr, size)) > 0) {
        ptr = buf;
        ptr += n;
    }
    if (n == -1) {
        fprintf(stderr, "read: %s", strerror(errno));
        exit(1);
    }

    String s = {
        .items = buf,
        .cap = size,
        .len = size,
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
