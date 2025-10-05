#pragma once

#include <stdlib.h>

#include "string.h"

typedef enum {
    T_IDENT = 1,
    T_KEYWORD,

    // Values
    T_STRING,
    T_NUMBER,

    // Symbols
    T_LPAREN,
    T_RPAREN,
    T_LBRACE,
    T_RBRACE,
    T_LBRACK,
    T_RBRACK,
    T_SEMICOLON,
    T_EQUAL,
    T_EQUALITY,
    T_NEQUALITY,
    T_MINUS,
    T_PLUS,
    T_SLASH,
    T_STAR,
    T_LT,
    T_LE,
    T_GT,
    T_GE,
    T_COMMA,
    T_LAND,
    T_LOR,

    T_EOF,
} TokenKind;

typedef struct {
    size_t line;
} Position;

typedef struct {
    TokenKind kind;
    String value;
    Position pos;
} Token;

typedef struct {
    size_t len;
    size_t cap;
    Token *items;
} Tokens;

typedef struct {
    Tokens array;
    size_t position;
} TokenIter;

TokenKind symbol_tokens[256];

char *symbol_values[256];

Tokens tokenise(const String *);

void print_tokens(const Tokens *);
