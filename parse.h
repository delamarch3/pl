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

typedef enum { BINARY_OP, VALUE, IDENT, CALL } ExprKind;
typedef struct Expr Expr;

typedef struct {
    size_t len;
    size_t cap;
    Expr *items;
} Exprs;

typedef enum { ADD, SUB, MUL, DIV, LT, LE, GT, GE } BinaryOp;
typedef struct {
    Expr *left;
    BinaryOp op;
    Expr *right;
} BinaryOpExpr;

typedef union {
    String str;
    uint64_t uint;
    int64_t sint;
} Value;

typedef enum { STRING, NUMBER, CHAR } ValueKind;
typedef struct {
    ValueKind kind;
    Value value;
} ValueExpr;

typedef struct {
    String name;
    Exprs args;
} CallExpr;

typedef union {
    BinaryOpExpr b;
    ValueExpr v;
    CallExpr c;
} ExprValue;

struct Expr {
    ExprKind kind;
    ExprValue value;
};

/* Statements */

typedef enum { DEFINITION, ASSIGNMENT, IF, WHILE } StatementKind;
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

typedef union {
    DefinitionStatement d;
    AssignmentStatement a;
    IfStatement i;
    WhileStatement w;
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
Statement parse_statement(TokenIter *);
Declaration parse_declaration(TokenIter *);
Expr parse_expr(TokenIter *);
