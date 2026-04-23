#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void parser_advance(Parser* parser) {
    parser->current = parser->next_token;
    parser->next_token = lexer_next(&parser->lexer);
    while (parser->next_token.type == TOKEN_NEWLINE) {
        token_free(&parser->next_token);
        parser->next_token = lexer_next(&parser->lexer);
    }
}

static ASTNode* parse_expression(Parser* parser);
static ASTNode* parse_statement(Parser* parser);

Parser parser_create(const char* source) {
    Parser parser = {.lexer = lexer_create(source)};
    parser.next_token = lexer_next(&parser.lexer);
    while (parser.next_token.type == TOKEN_NEWLINE) {
        token_free(&parser.next_token);
        parser.next_token = lexer_next(&parser.lexer);
    }
    parser_advance(&parser);
    return parser;
}

static ASTNode* node_alloc(NodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    return node;
}

static ASTNode* parse_primary(Parser* parser) {
    if (parser->current.type == TOKEN_INT_LITERAL) {
        ASTNode* node = node_alloc(NODE_INT_LITERAL);
        node->data.int_literal = atoi(parser->current.value);
        parser_advance(parser);
        return node;
    }

    if (parser->current.type == TOKEN_FLOAT_LITERAL) {
        ASTNode* node = node_alloc(NODE_FLOAT_LITERAL);
        node->data.float_literal = atof(parser->current.value);
        parser_advance(parser);
        return node;
    }

    if (parser->current.type == TOKEN_STRING_LITERAL) {
        ASTNode* node = node_alloc(NODE_STRING_LITERAL);
        node->data.string_literal = malloc(strlen(parser->current.value) + 1);
        strcpy(node->data.string_literal, parser->current.value);
        parser_advance(parser);
        return node;
    }

    if (parser->current.type == TOKEN_IDENTIFIER) {
        char* name = malloc(strlen(parser->current.value) + 1);
        strcpy(name, parser->current.value);
        parser_advance(parser);

        if (parser->current.type == TOKEN_LPAREN) {
            parser_advance(parser);
            ASTNode* node = node_alloc(NODE_FUNC_CALL);
            node->data.func_call.name = name;
            node->data.func_call.args = NULL;
            node->data.func_call.arg_count = 0;

            if (parser->current.type != TOKEN_RPAREN) {
                ASTNode* args = malloc(sizeof(ASTNode) * 16);
                memset(args, 0, sizeof(ASTNode) * 16);
                do {
                    if (parser->current.type == TOKEN_COMMA) parser_advance(parser);
                    args[node->data.func_call.arg_count++] = *parse_expression(parser);
                } while (parser->current.type == TOKEN_COMMA && node->data.func_call.arg_count < 16);
                node->data.func_call.args = args;
            }

            if (parser->current.type == TOKEN_RPAREN) parser_advance(parser);
            return node;
        }

        if (parser->current.type == TOKEN_LBRACKET) {
            parser_advance(parser);
            ASTNode* node = node_alloc(NODE_ARRAY_ACCESS);
            node->data.array_access.name = name;
            node->data.array_access.index = parse_expression(parser);
            if (parser->current.type == TOKEN_RBRACKET) parser_advance(parser);
            return node;
        }

        ASTNode* node = node_alloc(NODE_IDENTIFIER);
        node->data.identifier = name;
        return node;
    }

    if (parser->current.type == TOKEN_LPAREN) {
        parser_advance(parser);
        ASTNode* expr = parse_expression(parser);
        if (parser->current.type == TOKEN_RPAREN) parser_advance(parser);
        return expr;
    }

    if (parser->current.type == TOKEN_NOT || parser->current.type == TOKEN_MINUS) {
        ASTNode* node = node_alloc(NODE_UNARY_OP);
        node->data.unary_op.op = parser->current.value[0];
        parser_advance(parser);
        node->data.unary_op.operand = parse_primary(parser);
        return node;
    }

    return node_alloc(NODE_INT_LITERAL);
}

static ASTNode* parse_multiplicative(Parser* parser) {
    ASTNode* left = parse_primary(parser);

    while (parser->current.type == TOKEN_STAR || parser->current.type == TOKEN_SLASH ||
           parser->current.type == TOKEN_PERCENT) {
        ASTNode* node = node_alloc(NODE_BINARY_OP);
        node->data.binary_op.op = parser->current.value[0];
        node->data.binary_op.left = left;
        parser_advance(parser);
        node->data.binary_op.right = parse_primary(parser);
        left = node;
    }

    return left;
}

static ASTNode* parse_additive(Parser* parser) {
    ASTNode* left = parse_multiplicative(parser);

    while (parser->current.type == TOKEN_PLUS || parser->current.type == TOKEN_MINUS) {
        ASTNode* node = node_alloc(NODE_BINARY_OP);
        node->data.binary_op.op = parser->current.value[0];
        node->data.binary_op.left = left;
        parser_advance(parser);
        node->data.binary_op.right = parse_multiplicative(parser);
        left = node;
    }

    return left;
}

static ASTNode* parse_comparison(Parser* parser) {
    ASTNode* left = parse_additive(parser);

    while (parser->current.type == TOKEN_LT || parser->current.type == TOKEN_LTE ||
           parser->current.type == TOKEN_GT || parser->current.type == TOKEN_GTE ||
           parser->current.type == TOKEN_EQ || parser->current.type == TOKEN_NEQ) {
        ASTNode* node = node_alloc(NODE_BINARY_OP);
        node->data.binary_op.op = parser->current.value[0];
        if (parser->current.type == TOKEN_LTE) node->data.binary_op.op = 254;
        if (parser->current.type == TOKEN_GTE) node->data.binary_op.op = 255;
        if (parser->current.type == TOKEN_EQ) node->data.binary_op.op = '=';
        if (parser->current.type == TOKEN_NEQ) node->data.binary_op.op = '!';
        node->data.binary_op.left = left;
        parser_advance(parser);
        node->data.binary_op.right = parse_additive(parser);
        left = node;
    }

    return left;
}

static ASTNode* parse_logical_and(Parser* parser) {
    ASTNode* left = parse_comparison(parser);

    while (parser->current.type == TOKEN_AND) {
        ASTNode* node = node_alloc(NODE_BINARY_OP);
        node->data.binary_op.op = '&';
        node->data.binary_op.left = left;
        parser_advance(parser);
        node->data.binary_op.right = parse_comparison(parser);
        left = node;
    }

    return left;
}

static ASTNode* parse_logical_or(Parser* parser) {
    ASTNode* left = parse_logical_and(parser);

    while (parser->current.type == TOKEN_OR) {
        ASTNode* node = node_alloc(NODE_BINARY_OP);
        node->data.binary_op.op = '|';
        node->data.binary_op.left = left;
        parser_advance(parser);
        node->data.binary_op.right = parse_logical_and(parser);
        left = node;
    }

    return left;
}

static ASTNode* parse_expression(Parser* parser) {
    ASTNode* left = parse_logical_or(parser);

    if (parser->current.type == TOKEN_ASSIGN) {
        if (left->type == NODE_IDENTIFIER) {
            ASTNode* node = node_alloc(NODE_ASSIGNMENT);
            node->data.assignment.name = left->data.identifier;
            parser_advance(parser);
            node->data.assignment.expr = parse_expression(parser);
            free(left);
            return node;
        }
    }

    return left;
}

static ASTNode* parse_statement(Parser* parser) {
    if (parser->current.type == TOKEN_IF) {
        parser_advance(parser);
        if (parser->current.type == TOKEN_LPAREN) parser_advance(parser);
        ASTNode* node = node_alloc(NODE_IF_STMT);
        node->data.if_stmt.condition = parse_expression(parser);
        if (parser->current.type == TOKEN_RPAREN) parser_advance(parser);
        node->data.if_stmt.then_block = parse_statement(parser);
        if (parser->current.type == TOKEN_ELSE) {
            parser_advance(parser);
            node->data.if_stmt.else_block = parse_statement(parser);
        }
        return node;
    }

    if (parser->current.type == TOKEN_WHILE) {
        parser_advance(parser);
        if (parser->current.type == TOKEN_LPAREN) parser_advance(parser);
        ASTNode* node = node_alloc(NODE_WHILE_STMT);
        node->data.while_stmt.condition = parse_expression(parser);
        if (parser->current.type == TOKEN_RPAREN) parser_advance(parser);
        node->data.while_stmt.body = parse_statement(parser);
        return node;
    }

    if (parser->current.type == TOKEN_FOR) {
        parser_advance(parser);
        if (parser->current.type == TOKEN_LPAREN) parser_advance(parser);
        ASTNode* node = node_alloc(NODE_FOR_STMT);
        node->data.for_stmt.init = parse_expression(parser);
        if (parser->current.type == TOKEN_SEMICOLON) parser_advance(parser);
        node->data.for_stmt.condition = parse_expression(parser);
        if (parser->current.type == TOKEN_SEMICOLON) parser_advance(parser);
        node->data.for_stmt.increment = parse_expression(parser);
        if (parser->current.type == TOKEN_RPAREN) parser_advance(parser);
        node->data.for_stmt.body = parse_statement(parser);
        return node;
    }

    if (parser->current.type == TOKEN_LBRACE) {
        parser_advance(parser);
        ASTNode* node = node_alloc(NODE_BLOCK);
        ASTNode* head = NULL;
        ASTNode* tail = NULL;
        while (parser->current.type != TOKEN_RBRACE && parser->current.type != TOKEN_EOF) {
            ASTNode* stmt = parse_statement(parser);
            if (head == NULL) {
                head = stmt;
            } else {
                tail->next = stmt;
            }
            tail = stmt;
            if (parser->current.type == TOKEN_SEMICOLON) parser_advance(parser);
        }
        node->data.binary_op.left = head;
        if (parser->current.type == TOKEN_RBRACE) parser_advance(parser);
        return node;
    }

    if (parser->current.type == TOKEN_INT_KW || parser->current.type == TOKEN_FLOAT_KW ||
        parser->current.type == TOKEN_STRING_KW) {
        ASTNode* node = node_alloc(NODE_VAR_DECL);
        parser_advance(parser);
        if (parser->current.type == TOKEN_IDENTIFIER) {
            node->data.var_decl.name = malloc(strlen(parser->current.value) + 1);
            strcpy(node->data.var_decl.name, parser->current.value);
            parser_advance(parser);
            if (parser->current.type == TOKEN_ASSIGN) {
                parser_advance(parser);
                node->data.var_decl.init_expr = parse_expression(parser);
            }
        }
        if (parser->current.type == TOKEN_SEMICOLON) parser_advance(parser);
        return node;
    }

    if (parser->current.type == TOKEN_RETURN) {
        parser_advance(parser);
        ASTNode* node = node_alloc(NODE_RETURN_STMT);
        if (parser->current.type != TOKEN_SEMICOLON) {
            node->data.return_stmt.expr = parse_expression(parser);
        }
        if (parser->current.type == TOKEN_SEMICOLON) parser_advance(parser);
        return node;
    }

    if (parser->current.type == TOKEN_COUT) {
        parser_advance(parser);
        ASTNode* node = node_alloc(NODE_OUTPUT);
        if (parser->current.type == TOKEN_LSHIFT) {
            parser_advance(parser);
            node->data.output.expressions = malloc(sizeof(ASTNode) * 16);
            do {
                node->data.output.expressions[node->data.output.count++] = *parse_expression(parser);
                if (parser->current.type == TOKEN_LSHIFT) parser_advance(parser);
            } while (parser->current.type != TOKEN_SEMICOLON && parser->current.type != TOKEN_EOF);
        }
        if (parser->current.type == TOKEN_SEMICOLON) parser_advance(parser);
        return node;
    }

    ASTNode* expr = parse_expression(parser);
    if (parser->current.type == TOKEN_SEMICOLON) parser_advance(parser);
    return expr;
}

ASTNode* parser_parse(Parser* parser) {
    ASTNode* root = node_alloc(NODE_PROGRAM);
    ASTNode* head = NULL;
    ASTNode* tail = NULL;

    while (parser->current.type != TOKEN_EOF) {
        if (parser->current.type == TOKEN_USING) {
            parser_advance(parser);
            if (parser->current.type == TOKEN_NAMESPACE) parser_advance(parser);
            if (parser->current.type == TOKEN_STD) parser_advance(parser);
            if (parser->current.type == TOKEN_SEMICOLON) parser_advance(parser);
            continue;
        }

        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            if (head == NULL) {
                head = stmt;
            } else {
                tail->next = stmt;
            }
            tail = stmt;
        }
    }

    root->data.binary_op.left = head;
    return root;
}

void ast_node_free(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_IDENTIFIER:
            free(node->data.identifier);
            break;
        case NODE_STRING_LITERAL:
            free(node->data.string_literal);
            break;
        case NODE_VAR_DECL:
            free(node->data.var_decl.name);
            if (node->data.var_decl.init_expr) ast_node_free(node->data.var_decl.init_expr);
            break;
        case NODE_FUNC_CALL:
            free(node->data.func_call.name);
            if (node->data.func_call.args) {
                for (size_t i = 0; i < node->data.func_call.arg_count; i++) {
                    ast_node_free(&node->data.func_call.args[i]);
                }
                free(node->data.func_call.args);
            }
            break;
        case NODE_ASSIGNMENT:
            free(node->data.assignment.name);
            if (node->data.assignment.expr) ast_node_free(node->data.assignment.expr);
            break;
        case NODE_BINARY_OP:
            if (node->data.binary_op.left) ast_node_free(node->data.binary_op.left);
            if (node->data.binary_op.right) ast_node_free(node->data.binary_op.right);
            break;
        default:
            break;
    }

    if (node->next) ast_node_free(node->next);
    free(node);
}
