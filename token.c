#include <stdio.h>

#include "array.h"
#include "token.h"

TokenKind symbol_tokens[256] = {
    ['('] = T_LPAREN, [')'] = T_RPAREN, ['{'] = T_LBRACE, ['}'] = T_RBRACE, [';'] = T_SEMICOLON,
    ['='] = T_EQUAL,  ['-'] = T_MINUS,  ['+'] = T_PLUS,   ['/'] = T_SLASH,  ['*'] = T_STAR,
    ['<'] = T_LT,     ['>'] = T_GT,     [','] = T_COMMA,  ['['] = T_LBRACK, [']'] = T_RBRACK};

char *symbol_values[256] = {
    [T_LPAREN] = "(", [T_RPAREN] = ")", [T_LBRACE] = "{", [T_RBRACE] = "}", [T_SEMICOLON] = ";",
    [T_EQUAL] = "=",  [T_MINUS] = "-",  [T_PLUS] = "+",   [T_SLASH] = "/",  [T_STAR] = "*",
    [T_LT] = "<",     [T_GT] = ">",     [T_COMMA] = ",",  [T_LBRACK] = "[", [T_RBRACK] = "]"};

static int isnotdoublequote(char c) {
    return c != '"';
}

static int isalphabetic(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static int isnumeric(char c) {
    return c >= '0' && c <= '9';
}

static int isalphanumeric(char c) {
    return isalphabetic(c) || isnumeric(c);
}

static int isnewline(char c) {
    return c == '\n';
}

static int iswhitespace(char c) {
    return c == ' ' || c == '\t';
}

Tokens tokenise(const String *s) {
    Tokens toks = {0};
    size_t line = 1;

    for (size_t i = 0; i < s->len; i++) {
        char c = s->items[i];
        Token tok = {0};

        // TODO: comments
        if (iswhitespace(c)) {
            continue;
        } else if (isnewline(c)) {
            line++;
            continue;
        } else if (isalphabetic(c)) {
            String value = {0};
            string_extend_while(&value, s, i, isalphanumeric);
            i += value.len - 1;

            tok.kind = T_IDENT;
            tok.value = value;
        } else if (isnumeric(c)) {
            String value = {0};
            string_extend_while(&value, s, i, isnumeric);
            i += value.len - 1;

            tok.kind = T_NUMBER;
            tok.value = value;
        } else if (c == '-') {
            String value = string_from_cstr("-");
            string_extend_while(&value, s, i + 1, isnumeric);
            i += value.len - 1;

            if (value.len == 1) {
                tok.kind = T_MINUS;
            } else {
                tok.kind = T_NUMBER;
                tok.value = value;
            }
        } else if (c == '"') {
            String value = {0};
            string_extend_while(&value, s, i + 1, isnotdoublequote);
            i += value.len + 1;

            tok.kind = T_STRING;
            tok.value = value;
        } else {
            tok.kind = symbol_tokens[c];
            if (tok.kind == 0) {
                fprintf(stderr, "unexpected symbol: %c\n", c);
                exit(1);
            }
        }

        tok.pos.line = line;
        append(&toks, tok);
    }

    return toks;
}

Token *next_token(TokenState *ts) {
    if (ts->i == ts->toks.len) {
        return nullptr;
    }

    return &ts->toks.items[ts->i++];
}

Token *peek_token(TokenState *ts) {
    if (ts->i == ts->toks.len) {
        return nullptr;
    }

    return &ts->toks.items[ts->i];
}
