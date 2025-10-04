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

    for (size_t i = 0; i < tokens.len; i++) {
        Token t = tokens.items[i];
        printf("line %ld: ", t.pos.line);
        switch (t.kind) {
        case T_IDENT:
        case T_KEYWORD:
        case T_NUMBER:
        case T_STRING:
            printf("%.*s\n", (int)t.value.len, t.value.items);
            break;
        case T_EOF:
            break;
        default:
            printf("%s\n", symbol_values[t.kind]);
            break;
        }
    }

    TokenIter ts = {.array = tokens, .position = 0};
    Function func = parse_function(&ts);

    for (size_t i = 0; i < func.stmts.len; i++) {
        Statement stmt = func.stmts.items[i];
        switch (stmt.kind) {
        case S_DEFINITION:
            DefinitionStatement def = stmt.value.d;
            print_expr(&def.expr);
            printf("\n");
            break;
        case S_ASSIGNMENT:
            printf("assign\n");
            break;
        case S_IF:
            printf("if\n");
            break;
        case S_WHILE:
            printf("while\n");
            break;
        case S_RETURN:
            ReturnStatement ret = stmt.value.r;
            printf("return");
            print_expr(ret.expr);
            printf("\n");
        }
    }

    return 0;
}
