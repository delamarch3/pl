#pragma once

#include "str.h"

// program -> functions
//
// type -> ident | ident *
//
// decl -> type, ident
//
// func -> decl ( decls ) { stmts }
//
// stmt -> def
//         | expr
//         | if expr { stmts }
//         | while expr { stmts }
//
// expr -> ( expr )
//         | factor
//         | factor op factor
//         | ident ( exprs )
//
// factor -> expr
//         | ident
//         | value

typedef struct {
    String name;
    bool pointer;
} Type;

typedef struct {
    Type type;
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

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_EQY,
    OP_NEQY,
    OP_LAND,
    OP_LOR
} BinaryOp;
typedef struct {
    Expr *left;
    BinaryOp op;
    Expr *right;
} BinaryOpExpr;

typedef union {
    String str;
    String ch;
    long num;
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

typedef enum { S_DEFINITION, S_ASSIGN, S_EXPR, S_IF, S_WHILE, S_RETURN } StatementKind;
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
} AssignStatement;

typedef struct {
    Expr expr;
} ExprStatement;

typedef struct {
    Expr expr;
    Statements stmts;
} IfStatement;

typedef struct {
    Expr expr;
    Statements stmts;
} WhileStatement;

typedef struct {
    Expr *expr;
} ReturnStatement;

typedef union {
    DefinitionStatement d;
    AssignStatement a;
    ExprStatement e;
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

typedef struct {
    size_t len;
    size_t cap;
    Function *items;
} Functions;

/* Program */

typedef struct {
    Functions funcs;
} Program;
