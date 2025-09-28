#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_va_list.h>

#include "array.h"
#include "parse.h"
#include "token.h"

static Token expect(TokenIter *ts, TokenKind want) {
    Token *a = next(ts);
    if (a == nullptr) {
        fprintf(stderr, "unexpected eof\n");
        exit(1);
    }
    if (a->kind != want) {
        char *display;
        int len;
        if (a->value.items != nullptr) {
            display = a->value.items;
            len = a->value.len;
        } else {
            display = &symbol_values[a->kind];
            len = 1;
        }

        fprintf(stderr, "%ld: unexpected token: %.*s\n", a->pos.line, len, display);
        exit(1);
    }

    return *a;
}

static bool check(TokenIter *ts, TokenKind want) {
    Token *a = peek(ts);
    if (a == nullptr || a->kind != want) {
        return false;
    }

    next(ts);

    return true;
}

static bool checkn(TokenIter *ts, TokenKind start, ...) {
    size_t position = ts->position;

    va_list args;

    va_start(args, start);
    for (TokenKind want = start; want != 0; want = va_arg(args, TokenKind)) {
        Token *a = peek(ts);
        if (a == nullptr || a->kind != want) {
            ts->position = position;
            return false;
        }

        next(ts);
    }

    return true;
}

Function parse_function(TokenIter *ts) {
    Function func = {0};

    func.decl = parse_declaration(ts);

    expect(ts, T_LPAREN);
    if (check(ts, T_RPAREN)) {
        goto parse_statements;
    }

    Declarations args = {0};
    do {
        Declaration arg = parse_declaration(ts);
        append(&args, arg);
    } while (check(ts, T_COMMA));
    func.args = args;

    expect(ts, T_RPAREN);

parse_statements:
    Statements stmts = {0};

    expect(ts, T_LBRACE);

    while (true) {
        Statement stmt = {0};

        if (checkn(ts, T_IDENT, T_IDENT, 0)) {
            ts->position -= 2;

            stmt.kind = S_DEFINITION;

            DefinitionStatement *def = &stmt.value.d;
            def->decl = parse_declaration(ts);
            expect(ts, T_EQUAL);
            def->expr = parse_expr(ts);

            expect(ts, T_SEMICOLON);

            append(&stmts, stmt)
        } else if (checkn(ts, T_IDENT, T_EQUAL, 0)) {
            stmt.kind = S_ASSIGNMENT;

            expect(ts, T_SEMICOLON);
        } else {
            break;
        }
    }

    expect(ts, T_RBRACE);

    return func;
}

Declaration parse_declaration(TokenIter *ts) {
    Token type = expect(ts, T_IDENT);
    Token name = expect(ts, T_IDENT);

    Declaration decl = {
        .type = type.value,
        .name = name.value,
    };

    return decl;
}

Expr parse_expr(TokenIter *ts) {
    ts->position++;

    Expr expr = {0};
    expr.kind = E_VALUE;

    ValueExpr *value = &expr.value.v;
    value->kind = V_NUMBER;
    value->value.sint = 15;

    return expr;
}
