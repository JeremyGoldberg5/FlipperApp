#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

static bool is_identifier_start(char c) {
    return isalpha(c) || c == '_';
}

static bool is_identifier_char(char c) {
    return isalnum(c) || c == '_';
}

static char peek(Lexer* lexer) {
    if (lexer->pos >= lexer->length) return '\0';
    return lexer->source[lexer->pos];
}

static char peek_next(Lexer* lexer) {
    if (lexer->pos + 1 >= lexer->length) return '\0';
    return lexer->source[lexer->pos + 1];
}

static char advance(Lexer* lexer) {
    if (lexer->pos >= lexer->length) return '\0';
    char c = lexer->source[lexer->pos++];
    if (c == '\n') {
        lexer->line++;
        lexer->column = 0;
    } else {
        lexer->column++;
    }
    return c;
}

static void skip_whitespace(Lexer* lexer) {
    while (peek(lexer) && isspace(peek(lexer)) && peek(lexer) != '\n') {
        advance(lexer);
    }
}

static void skip_comment(Lexer* lexer) {
    if (peek(lexer) == '/' && peek_next(lexer) == '/') {
        while (peek(lexer) && peek(lexer) != '\n') {
            advance(lexer);
        }
    }
}

static Token read_number(Lexer* lexer) {
    size_t start = lexer->pos;
    bool is_float = false;

    while (isdigit(peek(lexer))) {
        advance(lexer);
    }

    if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
        is_float = true;
        advance(lexer);
        while (isdigit(peek(lexer))) {
            advance(lexer);
        }
    }

    Token token = {
        .type = is_float ? TOKEN_FLOAT_LITERAL : TOKEN_INT_LITERAL,
        .value = malloc(lexer->pos - start + 1),
        .length = lexer->pos - start,
        .line = lexer->line,
        .column = lexer->column,
    };
    strncpy(token.value, &lexer->source[start], token.length);
    token.value[token.length] = '\0';
    return token;
}

static Token read_string(Lexer* lexer, char quote) {
    advance(lexer);
    size_t start = lexer->pos;

    while (peek(lexer) && peek(lexer) != quote) {
        if (peek(lexer) == '\\') advance(lexer);
        advance(lexer);
    }

    Token token = {
        .type = TOKEN_STRING_LITERAL,
        .value = malloc(lexer->pos - start + 1),
        .length = lexer->pos - start,
        .line = lexer->line,
        .column = lexer->column,
    };
    strncpy(token.value, &lexer->source[start], token.length);
    token.value[token.length] = '\0';

    if (peek(lexer) == quote) advance(lexer);
    return token;
}

static Token read_identifier(Lexer* lexer) {
    size_t start = lexer->pos;

    while (is_identifier_char(peek(lexer))) {
        advance(lexer);
    }

    Token token = {
        .length = lexer->pos - start,
        .value = malloc(token.length + 1),
        .line = lexer->line,
        .column = lexer->column,
    };
    strncpy(token.value, &lexer->source[start], token.length);
    token.value[token.length] = '\0';

    if (strcmp(token.value, "int") == 0) token.type = TOKEN_INT_KW;
    else if (strcmp(token.value, "float") == 0) token.type = TOKEN_FLOAT_KW;
    else if (strcmp(token.value, "string") == 0) token.type = TOKEN_STRING_KW;
    else if (strcmp(token.value, "vector") == 0) token.type = TOKEN_VECTOR_KW;
    else if (strcmp(token.value, "if") == 0) token.type = TOKEN_IF;
    else if (strcmp(token.value, "else") == 0) token.type = TOKEN_ELSE;
    else if (strcmp(token.value, "for") == 0) token.type = TOKEN_FOR;
    else if (strcmp(token.value, "while") == 0) token.type = TOKEN_WHILE;
    else if (strcmp(token.value, "void") == 0) token.type = TOKEN_FUNCTION;
    else if (strcmp(token.value, "return") == 0) token.type = TOKEN_RETURN;
    else if (strcmp(token.value, "cout") == 0) token.type = TOKEN_COUT;
    else if (strcmp(token.value, "cin") == 0) token.type = TOKEN_CIN;
    else if (strcmp(token.value, "using") == 0) token.type = TOKEN_USING;
    else if (strcmp(token.value, "namespace") == 0) token.type = TOKEN_NAMESPACE;
    else if (strcmp(token.value, "std") == 0) token.type = TOKEN_STD;
    else token.type = TOKEN_IDENTIFIER;

    return token;
}

Lexer lexer_create(const char* source) {
    return (Lexer){
        .source = source,
        .pos = 0,
        .length = strlen(source),
        .line = 1,
        .column = 0,
    };
}

Token lexer_next(Lexer* lexer) {
    skip_whitespace(lexer);
    skip_comment(lexer);
    skip_whitespace(lexer);

    char c = peek(lexer);

    if (!c) {
        return (Token){.type = TOKEN_EOF, .line = lexer->line, .column = lexer->column};
    }

    if (c == '\n') {
        advance(lexer);
        return (Token){.type = TOKEN_NEWLINE, .line = lexer->line, .column = lexer->column};
    }

    if (isdigit(c)) {
        return read_number(lexer);
    }

    if (c == '"' || c == '\'') {
        return read_string(lexer, c);
    }

    if (is_identifier_start(c)) {
        return read_identifier(lexer);
    }

    advance(lexer);
    Token token = {.line = lexer->line, .column = lexer->column};

    switch (c) {
        case '+': token.type = TOKEN_PLUS; break;
        case '-': peek(lexer) == '>' ? (advance(lexer), token.type = TOKEN_ARROW) : (token.type = TOKEN_MINUS); break;
        case '*': token.type = TOKEN_STAR; break;
        case '/': token.type = TOKEN_SLASH; break;
        case '%': token.type = TOKEN_PERCENT; break;
        case '=': peek(lexer) == '=' ? (advance(lexer), token.type = TOKEN_EQ) : (token.type = TOKEN_ASSIGN); break;
        case '!': peek(lexer) == '=' ? (advance(lexer), token.type = TOKEN_NEQ) : (token.type = TOKEN_NOT); break;
        case '<':
            if (peek(lexer) == '=') {
                advance(lexer);
                token.type = TOKEN_LTE;
            } else if (peek(lexer) == '<') {
                advance(lexer);
                token.type = TOKEN_LSHIFT;
            } else {
                token.type = TOKEN_LT;
            }
            break;
        case '>':
            if (peek(lexer) == '=') {
                advance(lexer);
                token.type = TOKEN_GTE;
            } else if (peek(lexer) == '>') {
                advance(lexer);
                token.type = TOKEN_RSHIFT;
            } else {
                token.type = TOKEN_GT;
            }
            break;
        case '&': peek(lexer) == '&' ? (advance(lexer), token.type = TOKEN_AND) : (token.type = TOKEN_ERROR); break;
        case '|': peek(lexer) == '|' ? (advance(lexer), token.type = TOKEN_OR) : (token.type = TOKEN_ERROR); break;
        case '(': token.type = TOKEN_LPAREN; break;
        case ')': token.type = TOKEN_RPAREN; break;
        case '{': token.type = TOKEN_LBRACE; break;
        case '}': token.type = TOKEN_RBRACE; break;
        case '[': token.type = TOKEN_LBRACKET; break;
        case ']': token.type = TOKEN_RBRACKET; break;
        case ';': token.type = TOKEN_SEMICOLON; break;
        case ',': token.type = TOKEN_COMMA; break;
        case '.': token.type = TOKEN_DOT; break;
        case ':': token.type = TOKEN_COLON; break;
        default: token.type = TOKEN_ERROR;
    }

    token.value = malloc(2);
    token.value[0] = c;
    token.value[1] = '\0';
    token.length = 1;

    return token;
}

void token_free(Token* token) {
    if (token && token->value) {
        free(token->value);
        token->value = NULL;
    }
}

const char* token_type_str(TokenType type) {
    switch (type) {
        case TOKEN_INT_LITERAL: return "INT_LITERAL";
        case TOKEN_FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TOKEN_STRING_LITERAL: return "STRING_LITERAL";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INT_KW: return "int";
        case TOKEN_FLOAT_KW: return "float";
        case TOKEN_STRING_KW: return "string";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_FOR: return "for";
        case TOKEN_WHILE: return "while";
        case TOKEN_RETURN: return "return";
        case TOKEN_COUT: return "cout";
        case TOKEN_CIN: return "cin";
        case TOKEN_EOF: return "EOF";
        case TOKEN_NEWLINE: return "NEWLINE";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
