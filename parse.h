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

/* Expressions */

typedef enum { BINARY_OP, VALUE, IDENT, CALL } ExprType;
typedef struct {
    ExprType etype;
} Expr;

typedef struct {
    size_t len;
    size_t cap;
    Expr **items;
} Exprs;

typedef enum { ADD, SUB, MUL, DIV, LT, LE, GT, GE } BinaryOp;
typedef struct {
    ExprType etype;

    Expr *left;
    BinaryOp op;
    Expr *right;
} BinaryOpExpr;

typedef union {
    String str;
    uint64_t uint;
    int64_t sint;
} Value;

typedef enum { STRING, NUMBER, CHAR } ValueType;
typedef struct {
    ExprType etype;

    ValueType vtype;
    Value value;
} ValueExpr;

typedef struct {
    ExprType etype;

    String name;
    Exprs args;
} CallExpr;

/* Statements */

typedef struct {
    String type;
    String name;
} Declaration;

typedef struct {
    size_t len;
    size_t cap;
    Declaration *items;
} Declarations;

typedef enum { FUNCTION, DEFINITION, ASSIGNMENT, IF, WHILE } StatementType;
typedef struct {
    StatementType stype;
} Statement;

typedef struct {
    size_t len;
    size_t cap;
    Statement **items;
} Statements;

typedef struct {
    StatementType stype;

    Declaration decl;
    Declarations args;
} FunctionStatement;

typedef struct {
    StatementType stype;

    Declaration decl;
    Expr expr;
} DefinitionStatement;

typedef struct {
    StatementType stype;

    String name;
    Expr expr;
} AssignmentStatement;

typedef struct {
    StatementType stype;

    Expr expr;
    Statements statements;
} IfStatement;

typedef struct {
    StatementType stype;

    Expr expr;
    Statements statements;
} WhileStatement;

Declaration parse_decl(TokenIter *);
