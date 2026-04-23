#include "executor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

Executor* executor_create(void) {
    Executor* exec = malloc(sizeof(Executor));
    memset(exec, 0, sizeof(Executor));
    return exec;
}

void executor_free(Executor* executor) {
    if (!executor) return;
    for (size_t i = 0; i < executor->var_count; i++) {
        value_free(&executor->variables[i].value);
    }
    free(executor);
}

static Variable* find_variable(Executor* exec, const char* name) {
    for (size_t i = 0; i < exec->var_count; i++) {
        if (strcmp(exec->variables[i].name, name) == 0) {
            return &exec->variables[i];
        }
    }
    return NULL;
}

static void set_variable(Executor* exec, const char* name, Value value) {
    Variable* var = find_variable(exec, name);
    if (var) {
        value_free(&var->value);
        var->value = value;
    } else if (exec->var_count < MAX_VARIABLES) {
        strncpy(exec->variables[exec->var_count].name, name, 63);
        exec->variables[exec->var_count].value = value;
        exec->var_count++;
    }
}

static void output_append(Executor* exec, const char* str) {
    if (!str) return;
    size_t len = strlen(str);
    if (exec->output_len + len < MAX_OUTPUT_LEN) {
        strcpy(&exec->output[exec->output_len], str);
        exec->output_len += len;
    }
}

Value executor_eval_expr(Executor* executor, ASTNode* node) {
    if (!node) return value_null();

    switch (node->type) {
        case NODE_INT_LITERAL:
            return value_int(node->data.int_literal);

        case NODE_FLOAT_LITERAL:
            return value_float(node->data.float_literal);

        case NODE_STRING_LITERAL:
            return value_string(node->data.string_literal);

        case NODE_IDENTIFIER: {
            Variable* var = find_variable(executor, node->data.identifier);
            if (var) return value_copy(&var->value);
            return value_null();
        }

        case NODE_ARRAY_ACCESS: {
            Variable* var = find_variable(executor, node->data.array_access.name);
            if (var && var->value.type == VALUE_ARRAY) {
                Value idx = executor_eval_expr(executor, node->data.array_access.index);
                int32_t i = value_to_int(&idx);
                if (i >= 0 && i < (int32_t)var->value.data.array_val.size) {
                    return value_copy(&var->value.data.array_val.elements[i]);
                }
                value_free(&idx);
            }
            return value_null();
        }

        case NODE_BINARY_OP: {
            Value left = executor_eval_expr(executor, node->data.binary_op.left);
            Value right = executor_eval_expr(executor, node->data.binary_op.right);
            Value result = value_null();

            switch (node->data.binary_op.op) {
                case '+': result = value_add(&left, &right); break;
                case '-': result = value_subtract(&left, &right); break;
                case '*': result = value_multiply(&left, &right); break;
                case '/': result = value_divide(&left, &right); break;
                case '%': result = value_modulo(&left, &right); break;
                case '<': result = value_int(value_less(&left, &right)); break;
                case '>': result = value_int(value_greater(&left, &right)); break;
                case 254: result = value_int(value_less_equal(&left, &right)); break;
                case 255: result = value_int(value_greater_equal(&left, &right)); break;
                case '=': result = value_int(value_equal(&left, &right)); break;
                case '!': result = value_int(!value_equal(&left, &right)); break;
                case '&': result = value_int(value_to_bool(&left) && value_to_bool(&right)); break;
                case '|': result = value_int(value_to_bool(&left) || value_to_bool(&right)); break;
            }

            value_free(&left);
            value_free(&right);
            return result;
        }

        case NODE_UNARY_OP: {
            Value operand = executor_eval_expr(executor, node->data.unary_op.operand);
            Value result = value_null();

            switch (node->data.unary_op.op) {
                case '-': result = value_int(-value_to_int(&operand)); break;
                case '!': result = value_int(!value_to_bool(&operand)); break;
            }

            value_free(&operand);
            return result;
        }

        case NODE_FUNC_CALL: {
            if (strcmp(node->data.func_call.name, "sqrt") == 0 && node->data.func_call.arg_count > 0) {
                Value arg = executor_eval_expr(executor, &node->data.func_call.args[0]);
                Value result = value_float(sqrtf(value_to_float(&arg)));
                value_free(&arg);
                return result;
            }
            if (strcmp(node->data.func_call.name, "sin") == 0 && node->data.func_call.arg_count > 0) {
                Value arg = executor_eval_expr(executor, &node->data.func_call.args[0]);
                Value result = value_float(sinf(value_to_float(&arg)));
                value_free(&arg);
                return result;
            }
            if (strcmp(node->data.func_call.name, "cos") == 0 && node->data.func_call.arg_count > 0) {
                Value arg = executor_eval_expr(executor, &node->data.func_call.args[0]);
                Value result = value_float(cosf(value_to_float(&arg)));
                value_free(&arg);
                return result;
            }
            if (strcmp(node->data.func_call.name, "abs") == 0 && node->data.func_call.arg_count > 0) {
                Value arg = executor_eval_expr(executor, &node->data.func_call.args[0]);
                Value result = value_int(abs(value_to_int(&arg)));
                value_free(&arg);
                return result;
            }
            return value_null();
        }

        default:
            return value_null();
    }
}

static void executor_exec_stmt(Executor* executor, ASTNode* node);

static void executor_exec_block(Executor* executor, ASTNode* node) {
    if (!node) return;
    ASTNode* stmt = node->data.binary_op.left;
    while (stmt) {
        executor_exec_stmt(executor, stmt);
        stmt = stmt->next;
    }
}

static void executor_exec_stmt(Executor* executor, ASTNode* node) {
    if (!node || executor->error) return;

    switch (node->type) {
        case NODE_VAR_DECL: {
            Value init = value_null();
            if (node->data.var_decl.init_expr) {
                init = executor_eval_expr(executor, node->data.var_decl.init_expr);
            }
            set_variable(executor, node->data.var_decl.name, init);
            break;
        }

        case NODE_ASSIGNMENT: {
            Value val = executor_eval_expr(executor, node->data.assignment.expr);
            set_variable(executor, node->data.assignment.name, val);
            break;
        }

        case NODE_IF_STMT: {
            Value cond = executor_eval_expr(executor, node->data.if_stmt.condition);
            if (value_to_bool(&cond)) {
                executor_exec_stmt(executor, node->data.if_stmt.then_block);
            } else if (node->data.if_stmt.else_block) {
                executor_exec_stmt(executor, node->data.if_stmt.else_block);
            }
            value_free(&cond);
            break;
        }

        case NODE_WHILE_STMT: {
            while (true) {
                Value cond = executor_eval_expr(executor, node->data.while_stmt.condition);
                bool should_continue = value_to_bool(&cond);
                value_free(&cond);
                if (!should_continue) break;
                executor_exec_stmt(executor, node->data.while_stmt.body);
            }
            break;
        }

        case NODE_FOR_STMT: {
            executor_eval_expr(executor, node->data.for_stmt.init);
            while (true) {
                Value cond = executor_eval_expr(executor, node->data.for_stmt.condition);
                bool should_continue = value_to_bool(&cond);
                value_free(&cond);
                if (!should_continue) break;
                executor_exec_stmt(executor, node->data.for_stmt.body);
                executor_eval_expr(executor, node->data.for_stmt.increment);
            }
            break;
        }

        case NODE_BLOCK:
            executor_exec_block(executor, node);
            break;

        case NODE_OUTPUT: {
            for (size_t i = 0; i < node->data.output.count; i++) {
                Value val = executor_eval_expr(executor, &node->data.output.expressions[i]);
                char* str = value_to_string(&val);
                output_append(executor, str);
                free(str);
                value_free(&val);
            }
            output_append(executor, "\n");
            break;
        }

        case NODE_EXPR_STMT: {
            Value val = executor_eval_expr(executor, node);
            value_free(&val);
            break;
        }

        default:
            executor_eval_expr(executor, node);
    }
}

void executor_execute(Executor* executor, ASTNode* ast) {
    executor->error = false;
    executor->output_len = 0;
    executor->output[0] = '\0';

    if (!ast) return;

    executor_exec_block(executor, ast);
}

const char* executor_get_output(Executor* executor) {
    return executor->output;
}

const char* executor_get_error(Executor* executor) {
    return executor->error_msg;
}
