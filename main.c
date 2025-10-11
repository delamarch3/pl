#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gen.h"
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

    gen_program(&prg);

    return 0;
}
