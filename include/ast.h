#pragma once

#include "str.h"

// program -> functions | records
//
// type -> ident | ident *
//
// decl -> type, ident
//
// func -> fn decl ( decls ) { stmts }
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
//
// record -> record ident { decls }

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

typedef enum { ExprKindBinaryOp, ExprKindValue, ExprKindIdent, ExprKindCall } ExprKind;
typedef struct Expr Expr;

typedef struct {
    size_t len;
    size_t cap;
    Expr *items;
} Exprs;

typedef enum {
    BinaryOpAdd,
    BinaryOpSub,
    BinaryOpMul,
    BinaryOpDiv,
    BinaryOpLt,
    BinaryOpLe,
    BinaryOpGt,
    BinaryOpGe,
    BinaryOpEqy,
    BinaryOpNEqy,
    BinaryOpLogAnd,
    BinaryOpLogOr,
    BinaryOpIndex,
    BinaryOpAccess
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

typedef enum { ValueKindString, ValueKindNumber, ValueKindChar } ValueKind;
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

typedef enum {
    StatementKindDefinition,
    StatementKindAssign,
    StatementKindExpr,
    StatementKindIf,
    StatementKindWhile,
    StatementKindReturn
} StatementKind;
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

/* Record */

typedef struct {
    String name;
    Declarations fields;
} Record;

typedef struct {
    size_t len;
    size_t cap;
    Record *items;
} Records;

/* Program */

typedef struct {
    Functions funcs;
    Records records;
} Program;
