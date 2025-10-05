#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"
#include "string.h"
#include "token.h"

int main() {
    int fd = open("test.txt", O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "open: %s", strerror(errno));
        exit(1);
    }

    String src = string_from_file(fd);
    Tokens tokens = tokenise(&src);

    TokenIter ts = {.array = tokens, .position = 0};
    Program prg = parse_program(&ts);

    for (size_t i = 0; i < prg.funcs.len; i++) {
        Function *func = &prg.funcs.items[i];
        print_statements(&func->stmts, 0);
    }

    return 0;
}
