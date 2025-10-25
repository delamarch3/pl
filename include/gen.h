#include <stdio.h>

#include "ast.h"

typedef struct ExprContext ExprContext;
typedef struct TypeInfo TypeInfo;

void gen_program(const Program *);
void gen_function(const Function *);
void gen_statement(const TypeInfo *, const Statement *);
void gen_op(const char *, BinaryOp);
void gen_cmp_op(const char *, const char *);
void gen_logical_op(const char *, int);
void gen_expr(ExprContext *, const Expr *);
