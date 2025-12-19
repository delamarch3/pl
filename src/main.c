#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gen.h"
#include "parse.h"
#include "str.h"
#include "token.h"

int main() {
    int fd = open("examples/ops", O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "open: %s", strerror(errno));
        exit(1);
    }

    String src = string_from_file(fd);
    Tokens tokens = tokenise(&src);

    TokenIter ts = {.array = tokens, .position = 0};
    Program prg = parse_program(&ts);

    // for (size_t i = 0; i < prg.funcs.len; i++) {
    //     Function func = prg.funcs.items[i];
    //     printf("%.*s:\n", (int)func.decl.name.len, func.decl.name.items);

    //     print_statements(&func.stmts, 0);
    // }

    gen_program(&prg);

    return 0;
}
