#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "parse.h"
#include "token.h"
#include "util.h"

#define panic_unexpected_token(t)                                                                  \
    char *display;                                                                                 \
    if (t->value.items != nullptr) {                                                               \
        display = t->value.items;                                                                  \
        int len = t->value.len;                                                                    \
        fprintf(stderr, "%ld: unexpected token: %.*s\n", t->pos.line, len, display);               \
    } else {                                                                                       \
        display = symbol_values[t->kind];                                                          \
        fprintf(stderr, "%ld: unexpected token: %s\n", t->pos.line, display);                      \
    }                                                                                              \
    exit(1);

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

static bool checkkw(TokenIter *ts, const char *kw) {
    Token *a = peek(ts);
    if (a == nullptr || a->kind != T_KEYWORD || strcmp(kw, a->value.items) != 0) {
        return false;
    }

    next(ts);

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
            ts->position -= 2;

            stmt.kind = S_ASSIGNMENT;

            AssignmentStatement *as = &stmt.value.a;
            Token id = expect(ts, T_IDENT);
            as->name = id.value;
            expect(ts, T_EQUAL);
            as->expr = parse_expr(ts, 0);

            expect(ts, T_SEMICOLON);

            append(&stmts, stmt);
        } else if (checkkw(ts, "if")) {
            TODO("if statement");
        } else if (checkkw(ts, "while")) {
            TODO("while statement");
        } else if (checkkw(ts, "return")) {
            TODO("return statement");
        } else {
            break;
        }
    }
    expect(ts, T_RBRACE);
    func.stmts = stmts;

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

int next_prec(BinaryOp op) {
    switch (op) {
    case OP_GE:
    case OP_GT:
    case OP_LE:
    case OP_LT:
        return 8;
    case OP_ADD:
    case OP_SUB:
        return 9;
    case OP_MUL:
    case OP_DIV:
        return 10;
    default:
        fprintf(stderr, "uncountered unexpected op: %d", op);
        exit(1);
    }

    return 0;
}

Expr parse_expr(TokenIter *ts, int prec) {
    Expr expr = parse_prefix(ts);

    while (true) {
        Token *t = peek(ts);
        if (t == nullptr) {
            break;
        }

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
        case T_LT:
            op = OP_LT;
            break;
        case T_GT:
            op = OP_GT;
            break;
        case T_LE:
            op = OP_LE;
            break;
        case T_GE:
            op = OP_GE;
            break;
        default:
            goto done;
        }

        int nprec = next_prec(op);
        if (prec >= nprec) {
            break;
        }
        next(ts);

        Expr rhs = parse_expr(ts, prec);
        expr = binop(expr, op, rhs);
    }

done:
    return expr;
}

Expr parse_prefix(TokenIter *ts) {
    Token *t = next_token(ts);

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
    case T_IDENT:
        // TODO: call(e1, e2, ..)
        expr.kind = E_IDENT;
        IdentExpr *id = &expr.value.id;
        id->name = t->value;
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

char *display_op(BinaryOp op) {
    switch (op) {
    case OP_ADD:
        return "+";
    case OP_SUB:
        return "-";
    case OP_MUL:
        return "*";
    case OP_DIV:
        return "/";
    case OP_LT:
        return "<";
    case OP_LE:
        return "<=";
    case OP_GT:
        return ">";
    case OP_GE:
        return ">=";
    }
}

void print_expr(const Expr *expr) {
    switch (expr->kind) {
    case E_BINARY_OP:
        BinaryOpExpr b = expr->value.b;
        printf("(%s ", display_op(b.op));
        print_expr(b.left);
        printf(" ");
        print_expr(b.right);
        printf(")");
        break;
    case E_VALUE:
        ValueExpr v = expr->value.v;
        switch (v.kind) {
        case V_NUMBER:
            printf("%ld", v.value.num);
            break;
        case V_STRING:
            printf("%.*s", (int)v.value.str.len, v.value.str.items);
        case V_CHAR:
            printf("%c", v.value.ch);
        }
        break;
    case E_IDENT:
        IdentExpr id = expr->value.id;
        printf("%.*s", (int)id.name.len, id.name.items);
        break;
    case E_CALL:
        TODO("print call expr");
        break;
    }
}
