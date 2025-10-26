#include <stdio.h>

#include "ast.h"

typedef enum { Void, Byte, Char, Int, Long, Record } TypeKind;
typedef struct {
    TypeKind kind;
    int slotsize;
    char *opext;
    char *retext;
    bool pointer;
} TypeInfo;

typedef struct {
    TypeInfo *items;
    size_t len;
    size_t cap;
} TypeInfos;

typedef struct {
    const TypeInfo *type;
    bool settype;
} ExprContext;

// Builtin types
TypeInfo long_type = {.kind = Long, .slotsize = 2, .retext = ".d", .opext = ".d"};
TypeInfo int_type = {.kind = Int, .slotsize = 1, .retext = ".w", .opext = ".w"};
TypeInfo void_type = {.kind = Void, .slotsize = 0, .retext = "", .opext = ""};
TypeInfo char_type = {.kind = Char, .slotsize = 1, .retext = ".w", .opext = ".w"};
TypeInfo char_ptr_type = {
    .kind = Char, .slotsize = 1, .retext = ".w", .opext = ".w", .pointer = true};
TypeInfo byte_type = {.kind = Byte, .slotsize = 1, .retext = ".w", .opext = ".b"};

void gen_program(const Program *);

void gen_function(const Function *);

void gen_statement(const TypeInfo *, const Statement *);
void gen_assign_statement(const AssignStatement *);
void gen_expr_statement(const ExprStatement *);
void gen_definition_statement(const DefinitionStatement *);
void gen_if_statement(const TypeInfo *, const IfStatement *);
void gen_while_statement(const TypeInfo *, const WhileStatement *);
void gen_return_statement(const TypeInfo *, const ReturnStatement *);

void gen_expr(ExprContext *, const Expr *);
void gen_value_expr(ExprContext *, const ValueExpr *);
void gen_binary_op_expr(ExprContext *, const BinaryOpExpr *exp);
void gen_ident_expr(ExprContext *, const IdentExpr *);
void gen_call_expr(ExprContext *, const CallExpr *);

void gen_op(const char *, BinaryOp);
void gen_cmp_op(const char *, const char *);
void gen_logical_op(const char *, int);
