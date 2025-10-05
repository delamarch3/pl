#include <stdio.h>
#include <string.h>

#include "array.h"
#include "string.h"
#include "token.h"

#define panic_unexpected_symbol(c)                                                                 \
    if (c == nullptr) {                                                                            \
        fprintf(stderr, "unexpected eof\n");                                                       \
    }                                                                                              \
    fprintf(stderr, "unexpected symbol: %c\n", *c);                                                \
    exit(1);

TokenKind symbol_tokens[256] = {
    ['('] = T_LPAREN, [')'] = T_RPAREN, ['{'] = T_LBRACE, ['}'] = T_RBRACE, [';'] = T_SEMICOLON,
    ['='] = T_EQUAL,  ['-'] = T_MINUS,  ['+'] = T_PLUS,   ['/'] = T_SLASH,  ['*'] = T_STAR,
    ['<'] = T_LT,     ['>'] = T_GT,     [','] = T_COMMA,  ['['] = T_LBRACK, [']'] = T_RBRACK};

char *symbol_values[256] = {
    [T_LPAREN] = "(",    [T_RPAREN] = ")",    [T_LBRACE] = "{",     [T_RBRACE] = "}",
    [T_SEMICOLON] = ";", [T_EQUAL] = "=",     [T_MINUS] = "-",      [T_PLUS] = "+",
    [T_SLASH] = "/",     [T_STAR] = "*",      [T_LT] = "<",         [T_LE] = "<=",
    [T_GT] = ">",        [T_GE] = ">=",       [T_COMMA] = ",",      [T_LBRACK] = "[",
    [T_RBRACK] = "]",    [T_EQUALITY] = "==", [T_NEQUALITY] = "!=", [T_LAND] = "&&",
    [T_LOR] = "||"};

char *keywords[] = {"if", "else", "while", "for", "return", "null"};

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

static int isnotnewline(char c) {
    return c != '\n';
}

static int iswhitespace(char c) {
    return c == ' ' || c == '\t';
}

static void extend_while(String *s, CharIter *chars, int p(char)) {
    char *c;
    while ((c = peek(chars))) {
        if (!p(*c)) {
            break;
        }

        append(s, *c);

        if (++chars->position == chars->array.len) {
            break;
        }
    }
}

static void consume_while(CharIter *chars, int p(char)) {
    char *c;
    while ((c = peek(chars))) {
        if (!p(*c)) {
            break;
        }

        if (++chars->position == chars->array.len) {
            break;
        }
    }
}

Tokens tokenise(const String *s) {
    Tokens toks = {0};
    size_t line = 1;

    CharIter chars = {0};
    chars.array = *s;

    char *c;
    while ((c = peek(&chars))) {
        Token tok = {0};

        if (iswhitespace(*c)) {
            next(&chars);

            continue;
        } else if (isnewline(*c)) {
            next(&chars);
            line++;

            continue;
        } else if (isalphabetic(*c)) {
            String value = {0};
            extend_while(&value, &chars, isalphanumeric);

            tok.kind = T_IDENT;
            tok.value = value;

            for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
                if (strcmp(keywords[i], tok.value.items) == 0) {
                    tok.kind = T_KEYWORD;
                    break;
                }
            }
        } else if (isnumeric(*c)) {
            String value = {0};
            extend_while(&value, &chars, isnumeric);

            tok.kind = T_NUMBER;
            tok.value = value;
        } else if (*c == '/') {
            next(&chars);

            if (*(c = peek(&chars)) == '/') {
                consume_while(&chars, isnotnewline);
                continue;
            } else {
                tok.kind = T_SLASH;
            }
        } else if (*c == '-') {
            next(&chars);

            String value = string_from_cstr("-");
            extend_while(&value, &chars, isnumeric);

            if (value.len == 1) {
                tok.kind = T_MINUS;
            } else {
                tok.kind = T_NUMBER;
                tok.value = value;
            }
        } else if (*c == '"') {
            next(&chars);

            String value = {0};
            extend_while(&value, &chars, isnotdoublequote);

            char *n = next(&chars);
            if (n == nullptr || *n != '"') {
                fprintf(stderr, "expected closing quote: %c\n", *c);
                exit(1);
            }

            tok.kind = T_STRING;
            tok.value = value;
        } else if (*c == '<') {
            next(&chars);
            tok.kind = T_LT;

            char *n = peek(&chars);
            if (n != nullptr && *n == '=') {
                next(&chars);
                tok.kind = T_LE;
            }
        } else if (*c == '>') {
            next(&chars);
            tok.kind = T_GT;

            char *n = peek(&chars);
            if (n != nullptr && *n == '=') {
                next(&chars);
                tok.kind = T_GE;
            }
        } else if (*c == '&') {
            next(&chars);
            char *n = next(&chars);
            if (n == nullptr || *n != '&') {
                panic_unexpected_symbol(c);
            }

            tok.kind = T_LAND;
        } else if (*c == '|') {
            next(&chars);
            char *n = next(&chars);
            if (n == nullptr || *n != '|') {
                panic_unexpected_symbol(c);
            }

            tok.kind = T_LOR;
        } else if (*c == '!') {
            next(&chars);
            char *n = next(&chars);
            if (n == nullptr || *n != '=') {
                panic_unexpected_symbol(c);
            }

            tok.kind = T_NEQUALITY;
        } else if (*c == '=') {
            next(&chars);
            char *n = peek(&chars);
            if (n != nullptr && *n == '=') {
                next(&chars);
                tok.kind = T_EQUALITY;
            } else {
                tok.kind = T_EQUAL;
            }
        } else {
            next(&chars);

            tok.kind = symbol_tokens[*c];
            if (tok.kind == 0) {
                panic_unexpected_symbol(c);
            }
        }

        tok.pos.line = line;
        append(&toks, tok);
    }

    return toks;
}

void print_tokens(const Tokens *tokens) {
    if (tokens == nullptr) {
        return;
    }

    for (size_t i = 0; i < tokens->len; i++) {
        Token t = tokens->items[i];
        printf("line %ld: ", t.pos.line);
        switch (t.kind) {
        case T_IDENT:
        case T_KEYWORD:
        case T_NUMBER:
        case T_STRING:
            printf("%.*s\n", (int)t.value.len, t.value.items);
            break;
        case T_EOF:
            break;
        default:
            printf("%s\n", symbol_values[t.kind]);
            break;
        }
    }
}
