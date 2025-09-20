#include <stdio.h>
#include <stdlib.h>

#include "parse.h"
#include "token.h"

Declaration parse_decl(TokenState *ts) {
    Token *a = next_token(ts);
    if (a == nullptr) {
        fprintf(stderr, "unexpected eof");
        exit(1);
    }
    Token *b = next_token(ts);
    if (a == nullptr) {
        fprintf(stderr, "unexpected eof");
        exit(1);
    }

    String type = {0}, name = {0};

    switch (a->kind) {
    case T_IDENT:
        type = a->value;
    default:
        fprintf(stderr, "%ld: unexpected token: %d", a->pos.line, a->kind);
        exit(1);
    }

    switch (b->kind) {
    case T_IDENT:
        name = b->value;
    default:
        fprintf(stderr, "%ld: unexpected token: %d", b->pos.line, b->kind);
        exit(1);
    }

    Declaration decl = {
        .type = type,
        .name = name,
    };

    return decl;
}
