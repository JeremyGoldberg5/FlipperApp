#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_STRING,
    VALUE_ARRAY,
    VALUE_NULL,
} ValueType;

typedef struct Value {
    ValueType type;
    union {
        int32_t int_val;
        float float_val;
        char* string_val;
        struct {
            struct Value* elements;
            size_t size;
            size_t capacity;
        } array_val;
    } data;
} Value;

Value value_int(int32_t val);
Value value_float(float val);
Value value_string(const char* val);
Value value_array(void);
Value value_null(void);

void value_free(Value* v);
Value value_copy(const Value* v);

bool value_to_bool(const Value* v);
int32_t value_to_int(const Value* v);
float value_to_float(const Value* v);
char* value_to_string(const Value* v);

Value value_add(const Value* a, const Value* b);
Value value_subtract(const Value* a, const Value* b);
Value value_multiply(const Value* a, const Value* b);
Value value_divide(const Value* a, const Value* b);
Value value_modulo(const Value* a, const Value* b);

bool value_equal(const Value* a, const Value* b);
bool value_less(const Value* a, const Value* b);
bool value_less_equal(const Value* a, const Value* b);
bool value_greater(const Value* a, const Value* b);
bool value_greater_equal(const Value* a, const Value* b);
