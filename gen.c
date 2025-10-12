#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "gen.h"
#include "string.h"
#include "util.h"

typedef struct {
    String key;
    int local;
    int size;
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
SymbolMap smap = {0};
int locals = 0;

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

typedef enum { Void, Byte, Int, Long } TypeKind;
typedef struct {
    TypeKind kind;
    int slotsize;
} Type;

static Type get_type(const String *type) {
    Type t = {0};

    if (strcmp("int", type->items) == 0) {
        t.kind = Int;
        t.slotsize = 1;
    } else if (strcmp("long", type->items) == 0) {
        t.kind = Long;
        t.slotsize = 2;
    } else if (strcmp("void", type->items) == 0) {
        t.kind = Void;
        t.slotsize = 0;
    } else if (strcmp("char", type->items) == 0) {
        t.kind = Byte;
        t.slotsize = 1;
    } else {
        todo("unhandled return type");
    }

    return t;
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

    clear(&smap);
    locals = 0;
    for (size_t i = 0; i < args->len; i++) {
        Type type = get_type(&args->items[i].type);
        int local = locals;
        locals += type.slotsize;
        Symbol sym = {.key = args->items[i].name, .local = local, .size = type.slotsize};
        if (insert(&smap, sym) != nullptr) {
            panic("function argument redefined: %.*s\n", (int)sym.key.len, sym.key.items);
        }
    }

    char *ret;
    switch (get_type(&decl->type).kind) {
    case Void:
        ret = "ret";
        break;
    case Byte:
    case Int:
        ret = "ret.w";
        break;
    case Long:
        ret = "ret.d";
        break;
    }

    printf("%.*s:\n", (int)func->decl.name.len, func->decl.name.items);
    for (size_t i = 0; i < stmts->len; i++) {
        gen_statement(&stmts->items[i]);
    }
    printf("%s\n", ret);
}

void gen_statement(const Statement *stmt) {
    switch (stmt->kind) {
    case S_EXPR:
        const ExprStatement *estmt = &stmt->value.e;
        gen_expr(&estmt->expr);
        break;
    case S_DEFINITION:
        const DefinitionStatement *dstmt = &stmt->value.d;

        Type type = get_type(&dstmt->decl.type);
        int local = locals;
        locals += type.slotsize;
        Symbol sym = {.key = dstmt->decl.name, .local = local, .size = type.slotsize};
        if (insert(&smap, sym) != nullptr) {
            panic("variable redefined: %.*s\n", (int)sym.key.len, sym.key.items);
        }

        // SymbolInfo *test = get(&syms, &dstmt->decl.name);
        // printf("got symbol: %.*s\n", (int)test->key.len, test->key.items);

        gen_expr(&dstmt->expr);
        printf("store.w %d\n", sym.local);

        break;
    case S_IF:
        break;
    case S_WHILE:
        break;
    case S_RETURN:
        break;
    }
}

void gen_op(BinaryOp op) {
    switch (op) {
    case OP_ADD:
        printf("add\n");
        break;
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_LT:
    case OP_LE:
    case OP_GT:
    case OP_GE:
    case OP_ASN:
    case OP_EQY:
    case OP_NEQY:
    case OP_LAND:
    case OP_LOR:
        todo("gen op")
    }
}

void gen_expr(const Expr *expr) {
    switch (expr->kind) {
    case E_VALUE:
        switch (expr->value.v.kind) {
        case V_NUMBER:
            printf("push %ld\n", expr->value.v.value.num);
            break;
        case V_STRING:
        case V_CHAR:
            todo("string/char value translation");
            break;
        }
        break;
    case E_BINARY_OP:
        const BinaryOpExpr *b = &expr->value.b;
        gen_expr(b->left);
        gen_expr(b->right);
        gen_op(b->op);
        break;
    case E_IDENT:
        todo("ident expr translation");
        break;
    case E_CALL:
        todo("call expr translation");
        break;
    }
}
