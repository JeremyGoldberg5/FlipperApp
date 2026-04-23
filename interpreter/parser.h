#pragma once

#include "lexer.h"
#include <stddef.h>

typedef enum {
    NODE_PROGRAM,
    NODE_STATEMENT_LIST,
    NODE_VAR_DECL,
    NODE_FUNC_DECL,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_FOR_STMT,
    NODE_RETURN_STMT,
    NODE_EXPR_STMT,
    NODE_BLOCK,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_ASSIGNMENT,
    NODE_FUNC_CALL,
    NODE_ARRAY_ACCESS,
    NODE_ARRAY_INIT,
    NODE_INT_LITERAL,
    NODE_FLOAT_LITERAL,
    NODE_STRING_LITERAL,
    NODE_IDENTIFIER,
    NODE_OUTPUT,
    NODE_INPUT,
} NodeType;

typedef struct ASTNode {
    NodeType type;
    union {
        struct {
            char* name;
            struct ASTNode* type_node;
            struct ASTNode* init_expr;
        } var_decl;
        struct {
            char* name;
            struct ASTNode* params;
            struct ASTNode* body;
        } func_decl;
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_block;
            struct ASTNode* else_block;
        } if_stmt;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_stmt;
        struct {
            struct ASTNode* init;
            struct ASTNode* condition;
            struct ASTNode* increment;
            struct ASTNode* body;
        } for_stmt;
        struct {
            struct ASTNode* expr;
        } return_stmt;
        struct {
            struct ASTNode* expressions;
            size_t count;
        } output;
        struct {
            struct ASTNode* variable;
        } input;
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            char op;
        } binary_op;
        struct {
            struct ASTNode* operand;
            char op;
        } unary_op;
        struct {
            char* name;
            struct ASTNode* expr;
        } assignment;
        struct {
            char* name;
            struct ASTNode* args;
            size_t arg_count;
        } func_call;
        struct {
            char* name;
            struct ASTNode* index;
        } array_access;
        struct {
            struct ASTNode* elements;
            size_t count;
        } array_init;
        int32_t int_literal;
        float float_literal;
        char* string_literal;
        char* identifier;
    } data;
    struct ASTNode* next;
} ASTNode;

typedef struct {
    Token current;
    Token next_token;
    Lexer lexer;
} Parser;

Parser parser_create(const char* source);
ASTNode* parser_parse(Parser* parser);
void ast_node_free(ASTNode* node);
