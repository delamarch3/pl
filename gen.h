#include <stdio.h>

#include "ast.h"

void gen_program(const Program *);
void gen_function(const Function *);
void gen_statement(const Statement *);
void gen_op(BinaryOp);
void gen_expr(const Expr *);
