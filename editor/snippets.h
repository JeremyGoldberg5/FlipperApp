#pragma once

#include <stddef.h>

typedef struct {
    const char* name;
    const char* code;
} Snippet;

extern const Snippet snippets[];
extern const size_t snippets_count;
