#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gen.h"
#include "map.h"
#include "str.h"
#include "util.h"

static TypeInfo get_type(const Type *type) {
    if (type->pointer) {
        TypeInfo ptr_type = {
            .kind = Long, .slotsize = 2, .retext = ".d", .opext = ".d", .pointer = true};
        return ptr_type;
    } else if (strcmp("int", type->name.items) == 0) {
        return int_type;
    } else if (strcmp("long", type->name.items) == 0) {
        return long_type;
    } else if (strcmp("void", type->name.items) == 0) {
        return void_type;
    } else if (strcmp("char", type->name.items) == 0) {
        return char_type;
    } else if (strcmp("byte", type->name.items) == 0) {
        return byte_type;
    } else {
        todo("unhandled return type");
    }
}

// TODO
typedef enum { VariableSymbol, FunctionSymbol, RecordSymbol } SymbolKind;
typedef struct {
    String key;
    int local;
    TypeInfo type;
    SymbolKind kind;
} Symbol;

typedef struct {
    size_t len;
    size_t cap;
    Symbol *items;
} Symbols;

typedef struct {
    size_t cap;
    Symbols *items;
} SymbolMap;

// TODO: handle scope
// TODO: function names are symbols too
SymbolMap smap = {0};
int locals = 0;
int label = 0;
int strings = 0;

void gen_program(const Program *prg) {
    printf(".entry main\n\n");

    for (size_t i = 0; i < prg->funcs.len; i++) {
        gen_function(&prg->funcs.items[i]);
        printf("\n");
    }
}

void gen_function(const Function *func) {
    const Declaration *decl = &func->decl;
    const Declarations *args = &func->args;
    const Statements *stmts = &func->stmts;

    TypeInfo type = get_type(&decl->type);

    clear(&smap);
    locals = 0;
    for (size_t i = 0; i < args->len; i++) {
        TypeInfo type = get_type(&args->items[i].type);
        int local = locals;
        locals += type.slotsize;
        Symbol sym = {.key = args->items[i].name, .local = local, .type = type};

        if (get(&smap, &sym.key) != nullptr) {
            panic("function argument redefined: %.*s", (int)sym.key.len, sym.key.items);
        }

        insert(&smap, sym);
    }

    printf("%.*s:\n", (int)func->decl.name.len, func->decl.name.items);
    for (size_t i = 0; i < stmts->len; i++) {
        gen_statement(&type, &stmts->items[i]);
    }

    // TODO: check ret
}

void gen_statement(const TypeInfo *fntype, const Statement *stmt) {
    switch (stmt->kind) {
    case S_ASSIGN:
        gen_assign_statement(&stmt->value.a);
        break;
    case S_EXPR:
        gen_expr_statement(&stmt->value.e);
        break;
    case S_DEFINITION:
        gen_definition_statement(&stmt->value.d);
        break;
    case S_IF:
        gen_if_statement(fntype, &stmt->value.i);
        break;
    case S_WHILE:
        gen_while_statement(fntype, &stmt->value.w);
        break;
    case S_RETURN:
        gen_return_statement(fntype, &stmt->value.r);
        break;
    }
}

void gen_assign_statement(const AssignStatement *stmt) {
    ExprContext ctx = {0};

    Symbol *sym0 = get(&smap, &stmt->name);
    if (sym0 == nullptr) {
        panic("attempt to assign undeclared variable: %.*s", (int)stmt->name.len, stmt->name.items);
    }

    ctx.type = &sym0->type;
    gen_expr(&ctx, &stmt->expr);
    printf("store%s %d\n", sym0->type.opext, sym0->local);
}

void gen_expr_statement(const ExprStatement *estmt) {
    ExprContext ctx = {0};

    ctx.settype = true;
    gen_expr(&ctx, &estmt->expr);
}

void gen_definition_statement(const DefinitionStatement *stmt) {
    ExprContext ctx = {0};

    TypeInfo type = get_type(&stmt->decl.type);
    int local = locals;
    locals += type.slotsize;
    Symbol sym = {.key = stmt->decl.name, .local = local, .type = type};

    if (get(&smap, &sym.key) != nullptr) {
        panic("variable redefined: %.*s", (int)sym.key.len, sym.key.items);
    }

    insert(&smap, sym);

    ctx.type = &type;
    gen_expr(&ctx, &stmt->expr);
    printf("store%s %d\n", sym.type.opext, sym.local);
}

void gen_if_statement(const TypeInfo *fntype, const IfStatement *stmt) {
    ExprContext ctx = {0};

    ctx.settype = true;
    gen_expr(&ctx, &stmt->expr);

    char *opext = "";
    if (ctx.type != nullptr) {
        opext = ctx.type->opext;
    }

    int done = label++;
    printf("push%s 0\n", opext);
    printf("cmp%s\n", opext);
    printf("jmp.eq l%d\n", done);
    for (size_t i = 0; i < stmt->stmts.len; i++) {
        gen_statement(fntype, &stmt->stmts.items[i]);
    }

    printf("l%d:\n", done);
}

void gen_while_statement(const TypeInfo *fntype, const WhileStatement *stmt) {
    ExprContext ctx = {0};

    int start = label++;
    printf("l%d:\n", start);

    ctx.settype = true;
    gen_expr(&ctx, &stmt->expr);
    char *opext = "";
    if (ctx.type != nullptr) {
        opext = ctx.type->opext;
    }

    int done = label++;
    printf("push%s 0\n", opext);
    printf("cmp%s\n", opext);
    printf("jmp.eq l%d\n", done);
    for (size_t i = 0; i < stmt->stmts.len; i++) {
        gen_statement(fntype, &stmt->stmts.items[i]);
    }
    printf("jmp l%d\n", start);
    printf("l%d:\n", done);
}

void gen_return_statement(const TypeInfo *fntype, const ReturnStatement *stmt) {
    ExprContext ctx = {0};

    if (fntype->kind == Void && stmt->expr != nullptr) {
        panic("unexpected return expression, function type is void");
    }
    if (fntype->kind != Void && stmt->expr == nullptr) {
        panic("missing return expression, function type is not void");
    }

    ctx.type = fntype;
    if (stmt->expr != nullptr) {
        gen_expr(&ctx, stmt->expr);
    }

    printf("ret%s\n", fntype->retext);
}

void gen_op(const char *opext, BinaryOp op) {
    switch (op) {
    case OP_ADD:
        printf("add%s\n", opext);
        break;
    case OP_SUB:
        printf("sub%s\n", opext);
        break;
    case OP_MUL:
        printf("mul%s\n", opext);
        break;
    case OP_DIV:
        printf("div%s\n", opext);
        break;
    case OP_LT:
        gen_cmp_op(opext, "lt");
        break;
    case OP_LE:
        gen_cmp_op(opext, "le");
        break;
    case OP_GT:
        gen_cmp_op(opext, "gt");
        break;
    case OP_GE:
        gen_cmp_op(opext, "ge");
        break;
    case OP_EQY:
        gen_cmp_op(opext, "eq");
        break;
    case OP_NEQY:
        gen_cmp_op(opext, "ne");
        break;
    case OP_LAND:
        gen_logical_op(opext, 2);
        break;
    case OP_LOR:
        gen_logical_op(opext, 1);
        break;
    }
}

void gen_cmp_op(const char *opext, const char *jmpext) {
    int iftrue = label++;
    int cont = label++;
    printf("cmp%s\n", opext);
    printf("jmp.%s l%d\n", jmpext, iftrue);
    printf("push%s 0\n", opext);
    printf("jmp l%d\n", cont);
    printf("l%d:\n", iftrue);
    printf("push%s 1\n", opext);
    printf("l%d:\n", cont);
}

void gen_logical_op(const char *opext, int ntrue) {
    int iftrue = label++;
    int cont = label++;
    printf("add\n");
    printf("push%s %d\n", opext, ntrue);
    printf("cmp%s\n", opext);
    printf("jmp.ge l%d\n", iftrue);
    printf("push%s 0\n", opext);
    printf("jmp l%d\n", cont);
    printf("l%d:\n", iftrue);
    printf("push%s 1\n", opext);
    printf("l%d:\n", cont);
}

void gen_expr(ExprContext *ctx, const Expr *expr) {
    switch (expr->kind) {
    case E_VALUE:
        gen_value_expr(ctx, &expr->value.v);
        break;
    case E_BINARY_OP:
        gen_binary_op_expr(ctx, &expr->value.b);
        break;
    case E_IDENT:
        gen_ident_expr(ctx, &expr->value.id);
        break;
    case E_CALL:
        gen_call_expr(&expr->value.c);
        break;
    }
}

void gen_value_expr(const ExprContext *ctx, const ValueExpr *expr) {
    char *opext = ctx->type != nullptr ? ctx->type->opext : "";

    switch (expr->kind) {
    case V_NUMBER:
        printf("push%s %ld\n", opext, expr->value.num);
        break;
    case V_STRING:
        int s = strings++;
        printf(".data s%d .string \"%.*s\"\n", s, (int)expr->value.str.len, expr->value.str.items);
        printf("dataptr s%d\n", s);
        break;
    case V_CHAR:
        printf("push%s '%.*s'\n", opext, (int)expr->value.ch.len, expr->value.ch.items);
        break;
    }
}

void gen_binary_op_expr(ExprContext *ctx, const BinaryOpExpr *expr) {
    gen_expr(ctx, expr->left);
    gen_expr(ctx, expr->right);

    char *opext = ctx->type != nullptr ? ctx->type->opext : "";

    gen_op(opext, expr->op);
}

void gen_ident_expr(ExprContext *ctx, const IdentExpr *expr) {
    Symbol *sym = get(&smap, &expr->name);
    if (sym == nullptr) {
        panic("%.*s used before declaration", (int)expr->name.len, expr->name.items);
    }

    if (sym->type.kind == Void) {
        panic("cannot load value of type void");
    }

    if (ctx->type != nullptr && ctx->type->kind != sym->type.kind) {
        panic("type mismatch");
    }

    if (ctx->settype) {
        ctx->type = &sym->type;
        ctx->settype = false;
    }

    printf("load%s %d\n", sym->type.opext, sym->local);
}

void gen_call_expr(const CallExpr *expr) {
    // TODO: verify arg types - need to keep track of functions
    for (size_t i = 0; i < expr->args.len; i++) {
        ExprContext ctx = {0};
        ctx.settype = true;
        gen_expr(&ctx, &expr->args.items[i]);
    }

    printf("call %.*s\n", (int)expr->name.len, expr->name.items);
}
