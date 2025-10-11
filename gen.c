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

Symbol *insert(SymbolMap *syms, Symbol item) {
    if (syms->cap == 0) {
        syms->cap = 128;
        syms->items = calloc(syms->cap, sizeof(syms->items[0]));
    }

    size_t i = hash(&item.key) % 128;
    auto bucket = &syms->items[i];

    for (size_t i = 0; i < bucket->len; i++) {
        if (stringcmp(&bucket->items[i].key, &item.key) == 0) {
            bucket->items[i] = item;
            return &bucket->items[i];
        }
    }

    append(bucket, item);

    return nullptr;
}

Symbol *get(SymbolMap *syms, const String *key) {
    if (syms->cap == 0) {
        return nullptr;
    }

    size_t i = hash(key) % 128;
    auto bucket = &syms->items[i];

    for (size_t i = 0; i < bucket->len; i++) {
        if (stringcmp(&bucket->items[i].key, key) == 0) {
            return &bucket->items[i];
        }
    }

    return nullptr;
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
    const Declarations *args = &func->args; // TODO: keep track of locals
    const Statements *stmts = &func->stmts;

    char *ret;
    if (strcmp("int", decl->type.items) == 0) {
        ret = "ret.w";
    } else if (strcmp("long", decl->type.items) == 0) {
        ret = "ret.d";
    } else if (strcmp("void", decl->type.items) == 0) {
        ret = "ret";
    } else {
        todo("return type");
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

        // TODO: assuming int for now, but longs will take up two local slots
        Symbol sym = {.key = dstmt->decl.name, .local = locals++, .size = 1};
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
