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

static char *op_values[] = {
    [BinaryOpAdd] = "+",  [BinaryOpSub] = "-",   [BinaryOpMul] = "*",     [BinaryOpDiv] = "/",
    [BinaryOpLt] = "<",   [BinaryOpLe] = "<=",   [BinaryOpGt] = ">",      [BinaryOpGe] = ">=",
    [BinaryOpEqy] = "==", [BinaryOpNEqy] = "!=", [BinaryOpLogAnd] = "&&", [BinaryOpLogOr] = "||"};

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
    if (a == nullptr || a->kind != TokenKindKeyword || strcmp(kw, a->value.items) != 0) {
        return false;
    }

    next(ts);

    return true;
}

Program parse_program(TokenIter *ts) {
    Program prg = {0};

    do {
        if (checkkw(ts, "fn")) {
            Function func = parse_function(ts);
            append(&prg.funcs, func);
        } else if (checkkw(ts, "record")) {
            Record rec = parse_record(ts);
            append(&prg.records, rec);
        } else {
            Token *t = next(ts);
            panic_unexpected_token(t);
        }
    } while (peek(ts) != nullptr);

    return prg;
}

Function parse_function(TokenIter *ts) {
    Function func = {0};

    func.decl = parse_declaration(ts);

    expect(ts, TokenKindLParen);
    if (check(ts, TokenKindRParen)) {
        func.stmts = parse_statements(ts);
        return func;
    }

    do {
        Declaration arg = parse_declaration(ts);
        append(&func.args, arg);
    } while (check(ts, TokenKindComma));
    expect(ts, TokenKindRParen);

    func.stmts = parse_statements(ts);

    return func;
}

Record parse_record(TokenIter *ts) {
    Record rec = {0};

    rec.name = expect(ts, TokenKindIdent).value;

    expect(ts, TokenKindLBrace);
    do {
        Declaration field = parse_declaration(ts);
        append(&rec.fields, field);
    } while (check(ts, TokenKindComma));
    expect(ts, TokenKindRBrace);

    return rec;
}

Statement parse_statement(TokenIter *ts, bool *matched) {
    Statement stmt = {0};
    *matched = true;

    if (checkn(ts, TokenKindIdent, TokenKindIdent, 0) ||
        checkn(ts, TokenKindIdent, TokenKindStar, 0)) {
        ts->position -= 2;

        stmt.kind = StatementKindDefinition;

        DefinitionStatement *def = &stmt.value.d;
        def->decl = parse_declaration(ts);
        expect(ts, TokenKindEqual);
        def->expr = parse_expr(ts, 0);

        expect(ts, TokenKindSemicolon);
    } else if (checkn(ts, TokenKindIdent, TokenKindEqual, 0)) {
        ts->position -= 2;

        stmt.kind = StatementKindAssign;

        AssignStatement *asn = &stmt.value.a;
        Token id = expect(ts, TokenKindIdent);
        asn->name = id.value;
        expect(ts, TokenKindEqual);
        asn->expr = parse_expr(ts, 0);

        expect(ts, TokenKindSemicolon);
    } else if (checkkw(ts, "if")) {
        stmt.kind = StatementKindIf;

        IfStatement *ifs = &stmt.value.i;
        ifs->expr = parse_expr(ts, 0);
        ifs->stmts = parse_statements(ts);
    } else if (checkkw(ts, "while")) {
        stmt.kind = StatementKindWhile;

        WhileStatement *ws = &stmt.value.w;
        ws->expr = parse_expr(ts, 0);
        ws->stmts = parse_statements(ts);
    } else if (checkkw(ts, "return")) {
        stmt.kind = StatementKindReturn;

        ReturnStatement *ret = &stmt.value.r;

        if (!check(ts, TokenKindSemicolon)) {
            ret->expr = box(parse_expr(ts, 0));
            expect(ts, TokenKindSemicolon);
        }
    } else {
        Token *t = peek(ts);
        if (t == nullptr ||
            (t->kind != TokenKindIdent && t->kind != TokenKindLParen &&
             t->kind != TokenKindString && t->kind != TokenKindNumber)) { // parse_prefix
            *matched = false;
            return stmt;
        }

        stmt.kind = StatementKindExpr;

        ExprStatement *as = &stmt.value.e;
        as->expr = parse_expr(ts, 0);

        expect(ts, TokenKindSemicolon);
    }

    return stmt;
}

Statements parse_statements(TokenIter *ts) {
    Statements stmts = {0};

    expect(ts, TokenKindLBrace);
    while (true) {
        bool matched = false;
        Statement stmt = parse_statement(ts, &matched);
        if (!matched) {
            break;
        }

        append(&stmts, stmt);
    }
    expect(ts, TokenKindRBrace);

    return stmts;
}

Type parse_type(TokenIter *ts) {
    Type type = {0};

    type.name = expect(ts, TokenKindIdent).value;
    type.pointer = check(ts, TokenKindStar);

    return type;
}

Declaration parse_declaration(TokenIter *ts) {
    Declaration decl = {0};

    decl.type = parse_type(ts);
    decl.name = expect(ts, TokenKindIdent).value;

    return decl;
}

int next_prec(BinaryOp op) {
    switch (op) {
    case BinaryOpLogOr:
        return 2;
    case BinaryOpLogAnd:
        return 3;
    case BinaryOpEqy:
    case BinaryOpNEqy:
        return 4;
    case BinaryOpGe:
    case BinaryOpGt:
    case BinaryOpLe:
    case BinaryOpLt:
        return 8;
    case BinaryOpAdd:
    case BinaryOpSub:
        return 9;
    case BinaryOpMul:
    case BinaryOpDiv:
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
        case TokenKindPlus:
            op = BinaryOpAdd;
            break;
        case TokenKindMinus:
            op = BinaryOpSub;
            break;
        case TokenKindStar:
            op = BinaryOpMul;
            break;
        case TokenKindSlash:
            op = BinaryOpDiv;
            break;
        case TokenKindLt:
            op = BinaryOpLt;
            break;
        case TokenKindGt:
            op = BinaryOpGt;
            break;
        case TokenKindLe:
            op = BinaryOpLe;
            break;
        case TokenKindGe:
            op = BinaryOpGe;
            break;
        case TokenKindEquality:
            op = BinaryOpEqy;
            break;
        case TokenKindNEquality:
            op = BinaryOpNEqy;
            break;
        case TokenKindLogAnd:
            op = BinaryOpLogAnd;
            break;
        case TokenKindLogOr:
            op = BinaryOpLogOr;
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
    case TokenKindNumber:
        expr.kind = ExprKindValue;
        value = &expr.value.v;
        value->kind = ValueKindNumber;
        value->value.num = strtol(t->value.items, nullptr, 10);
        break;
    case TokenKindString:
        expr.kind = ExprKindValue;
        value = &expr.value.v;
        value->kind = ValueKindString;
        value->value.str = t->value;
        break;
    case TokenKindChar:
        expr.kind = ExprKindValue;
        value = &expr.value.v;
        value->kind = ValueKindChar;
        value->value.ch = t->value;
        break;
    case TokenKindLParen:
        expr = parse_expr(ts, 0);
        expect(ts, TokenKindRParen);
        break;
    case TokenKindIdent:
        Token *n = peek(ts);
        if (n == nullptr || n->kind != TokenKindLParen) {
            expr.kind = ExprKindIdent;
            IdentExpr *id = &expr.value.id;
            id->name = t->value;
        } else {
            expr.kind = ExprKindCall;
            CallExpr *c = &expr.value.c;
            c->name = t->value;

            expect(ts, TokenKindLParen);
            if (check(ts, TokenKindRParen)) {
                return expr;
            }

            do {
                Expr expr = parse_expr(ts, 0);
                append(&c->args, expr);
            } while (check(ts, TokenKindComma));
            expect(ts, TokenKindRParen);
        }

        break;
    default:
        panic_unexpected_token(t);
    }

    return expr;
}

Expr binop(Expr lhs, BinaryOp op, Expr rhs) {
    Expr expr = {0};
    expr.kind = ExprKindBinaryOp;
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
    case ExprKindBinaryOp:
        BinaryOpExpr b = expr->value.b;
        printf("(%s ", op_values[b.op]);
        print_expr(b.left);
        printf(" ");
        print_expr(b.right);
        printf(")");

        break;
    case ExprKindValue:
        ValueExpr v = expr->value.v;
        switch (v.kind) {
        case ValueKindNumber:
            printf("%ld", v.value.num);
            break;
        case ValueKindString:
            printf("\"%.*s\"", (int)v.value.str.len, v.value.str.items);
            break;
        case ValueKindChar:
            printf("'%.*s'", (int)v.value.ch.len, v.value.ch.items);
            break;
        }

        break;
    case ExprKindIdent:
        IdentExpr id = expr->value.id;
        printf("%.*s", (int)id.name.len, id.name.items);

        break;
    case ExprKindCall:
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
    case StatementKindDefinition:
        DefinitionStatement def = stmt->value.d;
        print_expr(&def.expr);
        break;
    case StatementKindAssign:
        // AssignStatement asn = stmt->value.a;
        todo("print assign");
        break;
    case StatementKindExpr:
        ExprStatement e = stmt->value.e;
        print_expr(&e.expr);
        break;
    case StatementKindIf:
        IfStatement ifs = stmt->value.i;
        printf("if ");
        print_expr(&ifs.expr);
        printf("\n");
        print_statements(&ifs.stmts, tab + 1);
        break;
    case StatementKindWhile:
        WhileStatement ws = stmt->value.w;
        printf("while ");
        print_expr(&ws.expr);
        printf("\n");
        print_statements(&ws.stmts, tab + 1);
        break;
    case StatementKindReturn:
        ReturnStatement ret = stmt->value.r;
        printf("return(");
        print_expr(ret.expr);
        printf(")");
    }
}
