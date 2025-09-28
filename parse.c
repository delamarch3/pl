#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "parse.h"
#include "token.h"

#define panic_unexpected_token(t)                                                                  \
    ({                                                                                             \
        char *display;                                                                             \
        int len;                                                                                   \
        if (t->value.items != nullptr) {                                                           \
            display = t->value.items;                                                              \
            len = t->value.len;                                                                    \
        } else {                                                                                   \
            display = &symbol_values[t->kind];                                                     \
            len = 1;                                                                               \
        }                                                                                          \
        fprintf(stderr, "%ld: unexpected token: %.*s\n", t->pos.line, len, display);               \
        exit(1);                                                                                   \
    })

#define box(x)                                                                                     \
    ({                                                                                             \
        auto ptr = malloc(sizeof(x));                                                              \
        if (ptr == nullptr) {                                                                      \
            fprintf(stderr, "malloc failed");                                                      \
            exit(1);                                                                               \
        }                                                                                          \
        memcpy(ptr, &x, sizeof(x));                                                                \
        ptr;                                                                                       \
    })

static Token *next_token(TokenIter *ts) {
    Token *t = next(ts);
    if (t == nullptr) {
        fprintf(stderr, "unexpected eof\n");
        exit(1);
    }

    return t;
}

static Token expect(TokenIter *ts, TokenKind want) {
    Token *t = next_token(ts);
    if (t->kind != want) {
        panic_unexpected_token(t);
    }

    return *t;
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
            def->expr = parse_expr(ts, 0);

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

int next_bp(TokenIter *ts) {
    return 0;
}

Expr parse_expr(TokenIter *ts, int bp) {
    Expr expr = parse_prefix(ts);

    while (true) {
        int nbp = next_bp(ts);
        if (bp >= nbp) {
            break;
        }

        Token *t = peek(ts);
        if (t == nullptr) {
            break;
        }
        next(ts);

        BinaryOp op;
        switch (t->kind) {
        case T_PLUS:
            op = OP_ADD;
            break;
        case T_MINUS:
            op = OP_SUB;
            break;
        case T_STAR:
            op = OP_MUL;
            break;
        case T_SLASH:
            op = OP_DIV;
            break;
        default:
            panic_unexpected_token(t);
        }

        Expr rhs = parse_expr(ts, bp);
        expr = binop(expr, op, rhs);
    }

    return expr;
}

Expr parse_prefix(TokenIter *ts) {
    Token *t = next_token(ts);

    // TODO: call(e1, e2, ..)
    ValueExpr *value;
    Expr expr = {0};
    switch (t->kind) {
    case T_NUMBER:
        expr.kind = E_VALUE;
        value = &expr.value.v;
        value->kind = V_NUMBER;
        value->value.num = strtol(t->value.items, nullptr, 10);
        break;
    case T_STRING:
        expr.kind = E_VALUE;
        value = &expr.value.v;
        value->kind = V_STRING;
        value->value.str = t->value;
        break;
    case T_LPAREN:
        expr = parse_expr(ts, 0);
        expect(ts, T_RPAREN);
        break;
    default:
        panic_unexpected_token(t);
    }

    return expr;
}

Expr binop(Expr lhs, BinaryOp op, Expr rhs) {
    Expr expr = {0};
    expr.kind = E_BINARY_OP;
    BinaryOpExpr *bop = &expr.value.b;
    bop->left = box(lhs);
    bop->op = op;
    bop->right = box(rhs);

    return expr;
}
