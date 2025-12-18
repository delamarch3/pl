#pragma once

#include <stdlib.h>

#include "str.h"

typedef enum {
    TokenKindIdent = 1,
    TokenKindKeyword,

    // Values
    TokenKindString,
    TokenKindChar,
    TokenKindNumber,

    // Symbols
    TokenKindLParen,
    TokenKindRParen,
    TokenKindLBrace,
    TokenKindRBrace,
    TokenKindLBrack,
    TokenKindRBrack,
    TokenKindSemicolon,
    TokenKindEqual,
    TokenKindEquality,
    TokenKindNEquality,
    TokenKindMinus,
    TokenKindPlus,
    TokenKindSlash,
    TokenKindStar,
    TokenKindLt,
    TokenKindLe,
    TokenKindGt,
    TokenKindGe,
    TokenKindComma,
    TokenKindLogAnd,
    TokenKindLogOr,
    TokenKindDot,

    TokenKindEof,
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
