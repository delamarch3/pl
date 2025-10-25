#pragma once

#include "ast.h"
#include "token.h"

Program parse_program(TokenIter *);
Function parse_function(TokenIter *);
Functions parse_functions(TokenIter *);
Statement parse_statement(TokenIter *, bool *);
Statements parse_statements(TokenIter *);
Type parse_type(TokenIter *);
Declaration parse_declaration(TokenIter *);

Expr parse_expr(TokenIter *, int);
Expr parse_prefix(TokenIter *);
int next_prec(BinaryOp);

Expr binop(Expr, BinaryOp, Expr);

void print_expr(const Expr *);
void print_statement(const Statement *, int);
void print_statements(const Statements *, int);
