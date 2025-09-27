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
typedef struct {
    ExprKind ekind;
} Expr;

typedef struct {
    size_t len;
    size_t cap;
    Expr **items;
} Exprs;

typedef enum { ADD, SUB, MUL, DIV, LT, LE, GT, GE } BinaryOp;
typedef struct {
    ExprKind ekind;

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
    ExprKind ekind;

    ValueKind vkind;
    Value value;
} ValueExpr;

typedef struct {
    ExprKind ekind;

    String name;
    Exprs args;
} CallExpr;

/* Statements */

typedef enum { DEFINITION, ASSIGNMENT, IF, WHILE } StatementKind;
typedef struct {
    StatementKind skind;
} Statement;

typedef struct {
    size_t len;
    size_t cap;
    Statement **items;
} Statements;

typedef struct {
    StatementKind skind;

    Declaration decl;
    Expr *expr;
} DefinitionStatement;

typedef struct {
    StatementKind skind;

    String name;
    Expr expr;
} AssignmentStatement;

typedef struct {
    StatementKind skind;

    Expr expr;
    Statements statements;
} IfStatement;

typedef struct {
    StatementKind skind;

    Expr expr;
    Statements statements;
} WhileStatement;

/* Function */

typedef struct {
    Declaration decl;
    Declarations args;
    Statements stmts;
} Function;

Function parse_function(TokenIter *);
Statement parse_statement(TokenIter *);
Declaration parse_declaration(TokenIter *);
Expr *parse_expr(TokenIter *);
