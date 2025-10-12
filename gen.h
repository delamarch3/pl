#include <stdio.h>

#include "ast.h"

typedef struct Context Context;
typedef struct Type Type;

void gen_program(const Program *);
void gen_function(const Function *);
void gen_statement(const Type *, const Statement *);
void gen_op(const char *, BinaryOp);
void gen_expr(const Type *, const Expr *);
