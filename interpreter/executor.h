#pragma once

#include "parser.h"
#include "value.h"
#include <stddef.h>

#define MAX_VARIABLES 256
#define MAX_FUNCTIONS 64
#define MAX_CALL_STACK 32
#define MAX_OUTPUT_LEN 2048

typedef struct {
    char name[64];
    Value value;
} Variable;

typedef struct {
    char name[64];
    ASTNode* params;
    ASTNode* body;
} Function;

typedef struct {
    Variable variables[MAX_VARIABLES];
    size_t var_count;
    Function functions[MAX_FUNCTIONS];
    size_t func_count;
    char output[MAX_OUTPUT_LEN];
    size_t output_len;
    bool error;
    char error_msg[256];
    int call_depth;
} Executor;

Executor* executor_create(void);
void executor_free(Executor* executor);
void executor_execute(Executor* executor, ASTNode* ast);
Value executor_eval_expr(Executor* executor, ASTNode* node);
const char* executor_get_output(Executor* executor);
const char* executor_get_error(Executor* executor);
