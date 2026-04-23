#include "value.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

Value value_int(int32_t val) {
    return (Value){.type = VALUE_INT, .data.int_val = val};
}

Value value_float(float val) {
    return (Value){.type = VALUE_FLOAT, .data.float_val = val};
}

Value value_string(const char* val) {
    Value v = {.type = VALUE_STRING};
    if (val) {
        v.data.string_val = malloc(strlen(val) + 1);
        strcpy(v.data.string_val, val);
    } else {
        v.data.string_val = malloc(1);
        v.data.string_val[0] = '\0';
    }
    return v;
}

Value value_array(void) {
    Value v = {.type = VALUE_ARRAY};
    v.data.array_val.elements = malloc(sizeof(Value) * 16);
    v.data.array_val.size = 0;
    v.data.array_val.capacity = 16;
    return v;
}

Value value_null(void) {
    return (Value){.type = VALUE_NULL};
}

void value_free(Value* v) {
    if (!v) return;
    if (v->type == VALUE_STRING && v->data.string_val) {
        free(v->data.string_val);
    } else if (v->type == VALUE_ARRAY && v->data.array_val.elements) {
        for (size_t i = 0; i < v->data.array_val.size; i++) {
            value_free(&v->data.array_val.elements[i]);
        }
        free(v->data.array_val.elements);
    }
}

Value value_copy(const Value* v) {
    Value copy = *v;
    if (v->type == VALUE_STRING) {
        copy.data.string_val = malloc(strlen(v->data.string_val) + 1);
        strcpy(copy.data.string_val, v->data.string_val);
    } else if (v->type == VALUE_ARRAY) {
        copy.data.array_val.elements = malloc(sizeof(Value) * v->data.array_val.capacity);
        copy.data.array_val.size = v->data.array_val.size;
        copy.data.array_val.capacity = v->data.array_val.capacity;
        for (size_t i = 0; i < v->data.array_val.size; i++) {
            copy.data.array_val.elements[i] = value_copy(&v->data.array_val.elements[i]);
        }
    }
    return copy;
}

bool value_to_bool(const Value* v) {
    switch (v->type) {
        case VALUE_INT:
            return v->data.int_val != 0;
        case VALUE_FLOAT:
            return v->data.float_val != 0.0f;
        case VALUE_STRING:
            return strlen(v->data.string_val) > 0;
        case VALUE_ARRAY:
            return v->data.array_val.size > 0;
        default:
            return false;
    }
}

int32_t value_to_int(const Value* v) {
    switch (v->type) {
        case VALUE_INT:
            return v->data.int_val;
        case VALUE_FLOAT:
            return (int32_t)v->data.float_val;
        case VALUE_STRING:
            return atoi(v->data.string_val);
        default:
            return 0;
    }
}

float value_to_float(const Value* v) {
    switch (v->type) {
        case VALUE_INT:
            return (float)v->data.int_val;
        case VALUE_FLOAT:
            return v->data.float_val;
        case VALUE_STRING:
            return atof(v->data.string_val);
        default:
            return 0.0f;
    }
}

char* value_to_string(const Value* v) {
    char* result = malloc(256);
    switch (v->type) {
        case VALUE_INT:
            snprintf(result, 256, "%d", v->data.int_val);
            break;
        case VALUE_FLOAT:
            snprintf(result, 256, "%f", v->data.float_val);
            break;
        case VALUE_STRING:
            strcpy(result, v->data.string_val);
            break;
        case VALUE_ARRAY:
            strcpy(result, "[array]");
            break;
        default:
            strcpy(result, "null");
    }
    return result;
}

Value value_add(const Value* a, const Value* b) {
    if (a->type == VALUE_STRING || b->type == VALUE_STRING) {
        char* as = value_to_string(a);
        char* bs = value_to_string(b);
        char result[512];
        snprintf(result, 512, "%s%s", as, bs);
        Value v = value_string(result);
        free(as);
        free(bs);
        return v;
    } else if (a->type == VALUE_FLOAT || b->type == VALUE_FLOAT) {
        return value_float(value_to_float(a) + value_to_float(b));
    } else {
        return value_int(value_to_int(a) + value_to_int(b));
    }
}

Value value_subtract(const Value* a, const Value* b) {
    if (a->type == VALUE_FLOAT || b->type == VALUE_FLOAT) {
        return value_float(value_to_float(a) - value_to_float(b));
    } else {
        return value_int(value_to_int(a) - value_to_int(b));
    }
}

Value value_multiply(const Value* a, const Value* b) {
    if (a->type == VALUE_FLOAT || b->type == VALUE_FLOAT) {
        return value_float(value_to_float(a) * value_to_float(b));
    } else {
        return value_int(value_to_int(a) * value_to_int(b));
    }
}

Value value_divide(const Value* a, const Value* b) {
    float bval = value_to_float(b);
    if (bval == 0.0f) {
        return value_int(0);
    }
    if (a->type == VALUE_FLOAT || b->type == VALUE_FLOAT) {
        return value_float(value_to_float(a) / bval);
    } else {
        return value_int(value_to_int(a) / value_to_int(b));
    }
}

Value value_modulo(const Value* a, const Value* b) {
    int32_t bval = value_to_int(b);
    if (bval == 0) return value_int(0);
    return value_int(value_to_int(a) % bval);
}

bool value_equal(const Value* a, const Value* b) {
    if (a->type != b->type) return false;
    switch (a->type) {
        case VALUE_INT:
            return a->data.int_val == b->data.int_val;
        case VALUE_FLOAT:
            return a->data.float_val == b->data.float_val;
        case VALUE_STRING:
            return strcmp(a->data.string_val, b->data.string_val) == 0;
        default:
            return false;
    }
}

bool value_less(const Value* a, const Value* b) {
    if (a->type == VALUE_FLOAT || b->type == VALUE_FLOAT) {
        return value_to_float(a) < value_to_float(b);
    } else if (a->type == VALUE_STRING && b->type == VALUE_STRING) {
        return strcmp(a->data.string_val, b->data.string_val) < 0;
    } else {
        return value_to_int(a) < value_to_int(b);
    }
}

bool value_less_equal(const Value* a, const Value* b) {
    return value_less(a, b) || value_equal(a, b);
}

bool value_greater(const Value* a, const Value* b) {
    return !value_less_equal(a, b);
}

bool value_greater_equal(const Value* a, const Value* b) {
    return !value_less(a, b);
}
