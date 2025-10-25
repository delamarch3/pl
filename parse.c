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
    if (t == nullptr) {                                                                            \
        fprintf(stderr, "unexpected eof\n");                                                       \
    } else if (t->value.items != nullptr) {                                                        \
        display = t->value.items;                                                                  \
        int len = t->value.len;                                                                    \
        fprintf(stderr, "%ld: unexpected token: %.*s\n", t->pos.line, len, display);               \
    } else {                                                                                       \
        display = symbol_values[t->kind];                                                          \
        fprintf(stderr, "%ld: unexpected token: %s\n", t->pos.line, display);                      \
    }                                                                                              \
    exit(1);

static char *op_values[] = {[OP_ADD] = "+",  [OP_SUB] = "-",   [OP_MUL] = "*",   [OP_DIV] = "/",
                            [OP_LT] = "<",   [OP_LE] = "<=",   [OP_GT] = ">",    [OP_GE] = ">=",
                            [OP_EQY] = "==", [OP_NEQY] = "!=", [OP_LAND] = "&&", [OP_LOR] = "||"};

static Token *next_token(TokenIter *ts) {
    Token *t = next(ts);
    if (t == nullptr) {
        panic_unexpected_token(t);
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

Program parse_program(TokenIter *ts) {
    Program prg = {0};

    prg.funcs = parse_functions(ts);

    return prg;
}

Function parse_function(TokenIter *ts) {
    Function func = {0};

    func.decl = parse_declaration(ts);

    expect(ts, T_LPAREN);
    if (check(ts, T_RPAREN)) {
        func.stmts = parse_statements(ts);
        return func;
    }

    do {
        Declaration arg = parse_declaration(ts);
        append(&func.args, arg);
    } while (check(ts, T_COMMA));
    expect(ts, T_RPAREN);

    func.stmts = parse_statements(ts);

    return func;
}

Functions parse_functions(TokenIter *ts) {
    Functions funcs = {0};

    do {
        Function func = parse_function(ts);
        append(&funcs, func);
    } while (peek(ts) != nullptr);

    return funcs;
}

Statement parse_statement(TokenIter *ts, bool *matched) {
    Statement stmt = {0};
    *matched = true;

    if (checkn(ts, T_IDENT, T_IDENT, 0) || checkn(ts, T_IDENT, T_STAR, 0)) {
        ts->position -= 2;

        stmt.kind = S_DEFINITION;

        DefinitionStatement *def = &stmt.value.d;
        def->decl = parse_declaration(ts);
        expect(ts, T_EQUAL);
        def->expr = parse_expr(ts, 0);

        expect(ts, T_SEMICOLON);
    } else if (checkn(ts, T_IDENT, T_EQUAL, 0)) {
        ts->position -= 2;

        stmt.kind = S_ASSIGN;

        AssignStatement *asn = &stmt.value.a;
        Token id = expect(ts, T_IDENT);
        asn->name = id.value;
        expect(ts, T_EQUAL);
        asn->expr = parse_expr(ts, 0);

        expect(ts, T_SEMICOLON);
    } else if (checkkw(ts, "if")) {
        stmt.kind = S_IF;

        IfStatement *ifs = &stmt.value.i;
        ifs->expr = parse_expr(ts, 0);
        ifs->stmts = parse_statements(ts);
    } else if (checkkw(ts, "while")) {
        stmt.kind = S_WHILE;

        WhileStatement *ws = &stmt.value.w;
        ws->expr = parse_expr(ts, 0);
        ws->stmts = parse_statements(ts);
    } else if (checkkw(ts, "return")) {
        stmt.kind = S_RETURN;

        ReturnStatement *ret = &stmt.value.r;

        if (!check(ts, T_SEMICOLON)) {
            ret->expr = box(parse_expr(ts, 0));
            expect(ts, T_SEMICOLON);
        }
    } else {
        Token *t = peek(ts);
        if (t == nullptr || (t->kind != T_IDENT && t->kind != T_LPAREN && t->kind != T_STRING &&
                             t->kind != T_NUMBER)) { // parse_prefix
            *matched = false;
            return stmt;
        }

        stmt.kind = S_EXPR;

        ExprStatement *as = &stmt.value.e;
        as->expr = parse_expr(ts, 0);

        expect(ts, T_SEMICOLON);
    }

    return stmt;
}

Statements parse_statements(TokenIter *ts) {
    Statements stmts = {0};

    expect(ts, T_LBRACE);
    while (true) {
        bool matched = false;
        Statement stmt = parse_statement(ts, &matched);
        if (!matched) {
            break;
        }

        append(&stmts, stmt);
    }
    expect(ts, T_RBRACE);

    return stmts;
}

Type parse_type(TokenIter *ts) {
    Type type = {0};

    type.name = expect(ts, T_IDENT).value;
    type.pointer = check(ts, T_STAR);

    return type;
}

Declaration parse_declaration(TokenIter *ts) {
    Declaration decl = {0};

    decl.type = parse_type(ts);
    decl.name = expect(ts, T_IDENT).value;

    return decl;
}

int next_prec(BinaryOp op) {
    switch (op) {
    case OP_LOR:
        return 2;
    case OP_LAND:
        return 3;
    case OP_EQY:
    case OP_NEQY:
        return 4;
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
        case T_EQUALITY:
            op = OP_EQY;
            break;
        case T_NEQUALITY:
            op = OP_NEQY;
            break;
        case T_LAND:
            op = OP_LAND;
            break;
        case T_LOR:
            op = OP_LOR;
            break;
        default:
            return expr;
        }

        int nprec = next_prec(op);
        if (prec >= nprec) {
            break;
        }
        next(ts);

        Expr rhs = parse_expr(ts, nprec);
        expr = binop(expr, op, rhs);
    }

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
    case T_CHAR:
        expr.kind = E_VALUE;
        value = &expr.value.v;
        value->kind = V_CHAR;
        value->value.ch = t->value;
        break;
    case T_LPAREN:
        expr = parse_expr(ts, 0);
        expect(ts, T_RPAREN);
        break;
    case T_IDENT:
        Token *n = peek(ts);
        if (n == nullptr || n->kind != T_LPAREN) {
            expr.kind = E_IDENT;
            IdentExpr *id = &expr.value.id;
            id->name = t->value;
        } else {
            expr.kind = E_CALL;
            CallExpr *c = &expr.value.c;
            c->name = t->value;

            expect(ts, T_LPAREN);
            if (check(ts, T_RPAREN)) {
                return expr;
            }

            do {
                Expr expr = parse_expr(ts, 0);
                append(&c->args, expr);
            } while (check(ts, T_COMMA));
            expect(ts, T_RPAREN);
        }

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

void print_expr(const Expr *expr) {
    if (expr == nullptr) {
        return;
    }

    switch (expr->kind) {
    case E_BINARY_OP:
        BinaryOpExpr b = expr->value.b;
        printf("(%s ", op_values[b.op]);
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
            printf("\"%.*s\"", (int)v.value.str.len, v.value.str.items);
            break;
        case V_CHAR:
            printf("'%.*s'", (int)v.value.ch.len, v.value.ch.items);
            break;
        }

        break;
    case E_IDENT:
        IdentExpr id = expr->value.id;
        printf("%.*s", (int)id.name.len, id.name.items);

        break;
    case E_CALL:
        char *sep = "";
        CallExpr c = expr->value.c;
        printf("%.*s(", (int)c.name.len, c.name.items);
        for (size_t i = 0; i < c.args.len; i++) {
            printf("%s", sep);
            print_expr(&c.args.items[i]);
            sep = ", ";
        }
        printf(")");

        break;
    }
}

void print_statements(const Statements *stmts, int tab) {
    if (stmts == nullptr) {
        return;
    }

    for (size_t i = 0; i < stmts->len; i++) {
        printf("%*s", tab * 4, "");
        print_statement(&stmts->items[i], tab);
        printf("\n");
    }
}

void print_statement(const Statement *stmt, int tab) {
    if (stmt == nullptr) {
        return;
    }

    switch (stmt->kind) {
    case S_DEFINITION:
        DefinitionStatement def = stmt->value.d;
        print_expr(&def.expr);
        break;
    case S_ASSIGN:
        // AssignStatement asn = stmt->value.a;
        todo("print assign");
        break;
    case S_EXPR:
        ExprStatement e = stmt->value.e;
        print_expr(&e.expr);
        break;
    case S_IF:
        IfStatement ifs = stmt->value.i;
        printf("if ");
        print_expr(&ifs.expr);
        printf("\n");
        print_statements(&ifs.stmts, tab + 1);
        break;
    case S_WHILE:
        WhileStatement ws = stmt->value.w;
        printf("while ");
        print_expr(&ws.expr);
        printf("\n");
        print_statements(&ws.stmts, tab + 1);
        break;
    case S_RETURN:
        ReturnStatement ret = stmt->value.r;
        printf("return(");
        print_expr(ret.expr);
        printf(")");
    }
}
