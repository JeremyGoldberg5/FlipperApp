#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum {
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_IDENTIFIER,

    TOKEN_INT_KW,
    TOKEN_FLOAT_KW,
    TOKEN_STRING_KW,
    TOKEN_VECTOR_KW,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_FUNCTION,
    TOKEN_RETURN,
    TOKEN_COUT,
    TOKEN_CIN,
    TOKEN_USING,
    TOKEN_NAMESPACE,
    TOKEN_STD,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_ASSIGN,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_LTE,
    TOKEN_GT,
    TOKEN_GTE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_COLON,
    TOKEN_ARROW,
    TOKEN_LSHIFT,
    TOKEN_RSHIFT,

    TOKEN_EOF,
    TOKEN_NEWLINE,
    TOKEN_ERROR,
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    size_t length;
    uint32_t line;
    uint32_t column;
} Token;

typedef struct {
    const char* source;
    size_t pos;
    size_t length;
    uint32_t line;
    uint32_t column;
} Lexer;

Lexer lexer_create(const char* source);
Token lexer_next(Lexer* lexer);
void token_free(Token* token);
const char* token_type_str(TokenType type);
