#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "gen.h"
#include "string.h"
#include "util.h"

char *jmpexts[] = {
    [OP_LT] = "lt", [OP_LE] = "le",  [OP_GT] = "gt",
    [OP_GE] = "ge", [OP_EQY] = "eq", [OP_NEQY] = "ne",
};

typedef enum { Void, Byte, Int, Long } TypeKind;
struct Type {
    TypeKind kind;
    int slotsize;
    char *opext;
    char *retext;
};

struct ExprContext {
    const Type *type;
    bool settype;
};

static Type get_type(const String *type) {
    Type t = {0};

    if (strcmp("int", type->items) == 0) {
        t.kind = Int;
        t.slotsize = 1;
        t.retext = t.opext = ".w";
    } else if (strcmp("long", type->items) == 0) {
        t.kind = Long;
        t.slotsize = 2;
        t.retext = t.opext = ".d";
    } else if (strcmp("void", type->items) == 0) {
        t.kind = Void;
        t.slotsize = 0;
        t.retext = t.opext = "";
    } else if (strcmp("char", type->items) == 0) {
        t.kind = Byte;
        t.slotsize = 1;
        t.opext = ".b";
        t.retext = ".w";
    } else {
        todo("unhandled return type");
    }

    return t;
}

typedef struct {
    String key;
    int local;
    Type type;
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

struct Context {
    Type fntype;
    Type *type;
};

// djb2 - http://www.cse.yorku.ca/~oz/hash.html
size_t hash(const String *s) {
    unsigned long hash = 5381;

    for (size_t i = 0; i < s->len; i++) {
        hash = ((hash << 5) + hash) + s->items[i]; /* hash * 33 + c */
    }

    return hash;
}

Symbol *insert(SymbolMap *smap, Symbol item) {
    if (smap->cap == 0) {
        smap->cap = 128;
        smap->items = calloc(smap->cap, sizeof(smap->items[0]));
    }

    size_t i = hash(&item.key) % 128;
    auto bucket = &smap->items[i];

    for (size_t i = 0; i < bucket->len; i++) {
        if (stringcmp(&bucket->items[i].key, &item.key) == 0) {
            bucket->items[i] = item;
            return &bucket->items[i];
        }
    }

    append(bucket, item);

    return nullptr;
}

Symbol *get(const SymbolMap *smap, const String *key) {
    if (smap->cap == 0) {
        return nullptr;
    }

    size_t i = hash(key) % 128;
    auto bucket = &smap->items[i];

    for (size_t i = 0; i < bucket->len; i++) {
        if (stringcmp(&bucket->items[i].key, key) == 0) {
            return &bucket->items[i];
        }
    }

    return nullptr;
}

void clear(SymbolMap *smap) {
    for (size_t i = 0; i < smap->cap; i++) {
        smap->items[i].len = 0;
    }
}

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

    Type type = get_type(&decl->type);

    clear(&smap);
    locals = 0;
    for (size_t i = 0; i < args->len; i++) {
        Type type = get_type(&args->items[i].type);
        int local = locals;
        locals += type.slotsize;
        Symbol sym = {.key = args->items[i].name, .local = local, .type = type};
        if (insert(&smap, sym) != nullptr) {
            panic("function argument redefined: %.*s", (int)sym.key.len, sym.key.items);
        }
    }

    printf("%.*s:\n", (int)func->decl.name.len, func->decl.name.items);
    for (size_t i = 0; i < stmts->len; i++) {
        gen_statement(&type, &stmts->items[i]);
    }
}

void gen_statement(const Type *fntype, const Statement *stmt) {
    int done;
    char *opext = "";
    ExprContext ctx = {0};

    // TODO: create functions for each case
    switch (stmt->kind) {
    case S_ASSIGN:
        const AssignStatement *asn = &stmt->value.a;

        Symbol *sym0 = get(&smap, &asn->name);
        if (sym0 == nullptr) {
            panic("attempt to assign undeclared variable: %.*s", (int)asn->name.len,
                  asn->name.items);
        }

        ctx.type = &sym0->type;
        gen_expr(&ctx, &asn->expr);
        printf("store%s %d\n", sym0->type.opext, sym0->local);

        break;
    case S_EXPR:
        const ExprStatement *estmt = &stmt->value.e;
        ctx.settype = true;
        gen_expr(&ctx, &estmt->expr);
        break;
    case S_DEFINITION:
        const DefinitionStatement *dstmt = &stmt->value.d;

        Type type = get_type(&dstmt->decl.type);
        int local = locals;
        locals += type.slotsize;
        Symbol sym = {.key = dstmt->decl.name, .local = local, .type = type};
        if (insert(&smap, sym) != nullptr) {
            panic("variable redefined: %.*s", (int)sym.key.len, sym.key.items);
        }

        // SymbolInfo *test = get(&syms, &dstmt->decl.name);
        // printf("got symbol: %.*s\n", (int)test->key.len, test->key.items);

        ctx.type = &type;
        gen_expr(&ctx, &dstmt->expr);
        printf("store%s %d\n", sym.type.opext, sym.local);

        break;
    case S_IF:
        const IfStatement *ifstmt = &stmt->value.i;
        ctx.settype = true;
        gen_expr(&ctx, &ifstmt->expr);

        char *opext = "";
        if (ctx.type != nullptr) {
            opext = ctx.type->opext;
        }

        int done = label++;
        printf("push%s 0\n", opext);
        printf("cmp%s\n", opext);
        printf("jmp.eq l%d\n", done);
        for (size_t i = 0; i < ifstmt->stmts.len; i++) {
            gen_statement(fntype, &ifstmt->stmts.items[i]);
        }
        printf("l%d:\n", done);

        break;
    case S_WHILE:
        const WhileStatement *wstmt = &stmt->value.w;

        int start = label++;
        printf("l%d:\n", start);

        ctx.settype = true;
        gen_expr(&ctx, &wstmt->expr);
        opext = "";
        if (ctx.type != nullptr) {
            opext = ctx.type->opext;
        }

        done = label++;
        printf("push%s 0\n", opext);
        printf("cmp%s\n", opext);
        printf("jmp.eq l%d\n", done);
        for (size_t i = 0; i < wstmt->stmts.len; i++) {
            gen_statement(fntype, &wstmt->stmts.items[i]);
        }
        printf("jmp l%d\n", start);
        printf("l%d:\n", done);

        break;
    case S_RETURN:
        const ReturnStatement *ret = &stmt->value.r;
        if (fntype->kind == Void && ret->expr != nullptr) {
            panic("unexpected return expression, function type is void");
        }
        if (fntype->kind != Void && ret->expr == nullptr) {
            panic("missing return expression, function type is not void");
        }

        ctx.type = fntype;
        if (ret->expr != nullptr) {
            gen_expr(&ctx, ret->expr);
        }

        printf("ret%s\n", fntype->retext);

        break;
    }
}

void gen_op(const char *opext, BinaryOp op) {
    int tru, done;
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
    case OP_LE:
    case OP_GT:
    case OP_GE:
    case OP_EQY:
    case OP_NEQY:
        char *jmpext = jmpexts[op];
        tru = label++;
        done = label++;
        printf("cmp%s\n", opext);
        printf("jmp.%s l%d\n", jmpext, tru);
        printf("push%s 0\n", opext);
        printf("jmp l%d\n", done);
        printf("l%d:\n", tru);
        printf("push%s 1\n", opext);
        printf("l%d:\n", done);
        break;
    case OP_LAND:
        tru = label++;
        done = label++;
        printf("add\n");
        printf("push%s 2\n", opext);
        printf("cmp%s\n", opext);
        printf("jmp.ge l%d\n", tru);
        printf("push%s 0\n", opext);
        printf("jmp l%d\n", done);
        printf("l%d:\n", tru);
        printf("push%s 1\n", opext);
        printf("l%d:\n", done);
        break;
    case OP_LOR:
        tru = label++;
        done = label++;
        printf("add\n");
        printf("push%s 1\n", opext);
        printf("cmp%s\n", opext);
        printf("jmp.ge l%d\n", tru);
        printf("push%s 0\n", opext);
        printf("jmp l%d\n", done);
        printf("l%d:\n", tru);
        printf("push%s 1\n", opext);
        printf("l%d:\n", done);
        break;
    }
}

void gen_cmp_op() {
    todo("gen_cmp_op");
}

void gen_logical_op() {
    todo("gen_logical_op");
}

void gen_expr(ExprContext *ctx, const Expr *expr) {
    char *opext = "";
    if (ctx->type != nullptr) {
        opext = ctx->type->opext;
    }

    switch (expr->kind) {
    case E_VALUE:
        switch (expr->value.v.kind) {
        case V_NUMBER:
            printf("push%s %ld\n", opext, expr->value.v.value.num);
            break;
        case V_STRING:
        case V_CHAR:
            todo("string/char value translation");
            break;
        }
        break;
    case E_BINARY_OP:
        const BinaryOpExpr *b = &expr->value.b;
        gen_expr(ctx, b->left);
        gen_expr(ctx, b->right);

        if (ctx->type != nullptr) {
            opext = ctx->type->opext;
        }

        gen_op(opext, b->op);
        break;
    case E_IDENT:
        const IdentExpr *id = &expr->value.id;

        Symbol *sym = get(&smap, &id->name);
        if (sym == nullptr) {
            panic("%.*s used before declaration", (int)id->name.len, id->name.items);
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

        break;
    case E_CALL:
        const CallExpr *call = &expr->value.c;

        // TODO: verify arg types
        for (size_t i = 0; i < call->args.len; i++) {
            ExprContext ctx = {0};
            ctx.settype = true;
            gen_expr(&ctx, &call->args.items[i]);
        }

        printf("call %.*s\n", (int)call->name.len, call->name.items);
        break;
    }
}
