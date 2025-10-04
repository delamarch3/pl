#pragma once

#include "string.h"
#include "token.h"

/*
 * declaration { ident, ident }

 * function {
 *     declration, declration*, statement*
 * }
 *
 * statement {
 *     definition {
 *         declration, expr
 *     }
 *     assignment {
 *         ident, expr
 *     }
 *     if {
 *         expr, statement*
 *     }
 *     while {
 *         expr, statement*
 *     }
 * }
 *
 * expr {
 *     value {
 *         string | number | char
 *     }
 *     binary {
 *         expr, op, expr
 *     }
 *     unary {
 *         op, expr
 *     }
 *     call {
 *         ident, expr*
 *     }
 * }
 */

typedef struct {
    String type;
    String name;
} Declaration;

typedef struct {
    size_t len;
    size_t cap;
    Declaration *items;
} Declarations;

/* Expressions */

typedef enum { E_BINARY_OP, E_VALUE, E_IDENT, E_CALL } ExprKind;
typedef struct Expr Expr;

typedef struct {
    size_t len;
    size_t cap;
    Expr *items;
} Exprs;

typedef enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_LT, OP_LE, OP_GT, OP_GE } BinaryOp;
typedef struct {
    Expr *left;
    BinaryOp op;
    Expr *right;
} BinaryOpExpr;

typedef union {
    String str;
    long num;
    char ch;
} Value;

typedef enum { V_STRING, V_NUMBER, V_CHAR } ValueKind;
typedef struct {
    ValueKind kind;
    Value value;
} ValueExpr;

typedef struct {
    String name;
} IdentExpr;

typedef struct {
    String name;
    Exprs args;
} CallExpr;

typedef union {
    BinaryOpExpr b;
    ValueExpr v;
    CallExpr c;
    IdentExpr id;
} ExprValue;

struct Expr {
    ExprKind kind;
    ExprValue value;
};

/* Statements */

typedef enum { S_DEFINITION, S_ASSIGNMENT, S_IF, S_WHILE, S_RETURN } StatementKind;
typedef struct Statement Statement;

typedef struct {
    size_t len;
    size_t cap;
    Statement *items;
} Statements;

typedef struct {
    Declaration decl;
    Expr expr;
} DefinitionStatement;

typedef struct {
    String name;
    Expr expr;
} AssignmentStatement;

typedef struct {
    Expr expr;
    Statements statements;
} IfStatement;

typedef struct {
    Expr expr;
    Statements statements;
} WhileStatement;

typedef struct {
    Expr *expr;
} ReturnStatement;

typedef union {
    DefinitionStatement d;
    AssignmentStatement a;
    IfStatement i;
    WhileStatement w;
    ReturnStatement r;
} StatementValue;

struct Statement {
    StatementKind kind;
    StatementValue value;
};

/* Function */

typedef struct {
    Declaration decl;
    Declarations args;
    Statements stmts;
} Function;

Function parse_function(TokenIter *);
Statement parse_statement(TokenIter *, bool *);
Statements parse_statements(TokenIter *);
Declaration parse_declaration(TokenIter *);

Expr parse_expr(TokenIter *, int);
Expr parse_prefix(TokenIter *);
Expr parse_infix(TokenIter *, Expr, int);

int next_prec(BinaryOp);

Expr binop(Expr, BinaryOp, Expr);

void print_expr(const Expr *);
