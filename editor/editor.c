#include "editor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

TextEditor* editor_alloc(void) {
    TextEditor* editor = malloc(sizeof(TextEditor));
    memset(editor, 0, sizeof(TextEditor));
    editor->lines[0] = malloc(MAX_LINE_LENGTH);
    editor->lines[0][0] = '\0';
    editor->line_count = 1;
    return editor;
}

void editor_free(TextEditor* editor) {
    if (!editor) return;
    for (size_t i = 0; i < editor->line_count; i++) {
        if (editor->lines[i]) {
            free(editor->lines[i]);
        }
    }
    free(editor);
}

void editor_clear(TextEditor* editor) {
    if (!editor) return;
    for (size_t i = 0; i < editor->line_count; i++) {
        if (editor->lines[i]) {
            free(editor->lines[i]);
        }
    }
    editor->line_count = 1;
    editor->lines[0] = malloc(MAX_LINE_LENGTH);
    editor->lines[0][0] = '\0';
    editor->cursor_line = 0;
    editor->cursor_col = 0;
    editor->scroll_line = 0;
    editor->scroll_col = 0;
}

void editor_load_text(TextEditor* editor, const char* text) {
    if (!editor || !text) return;
    editor_clear(editor);

    size_t line_num = 0;
    size_t col = 0;
    const char* ptr = text;

    while (*ptr && line_num < MAX_LINES) {
        if (*ptr == '\n') {
            editor->lines[line_num][col] = '\0';
            line_num++;
            col = 0;
            if (line_num < MAX_LINES) {
                editor->lines[line_num] = malloc(MAX_LINE_LENGTH);
                editor->lines[line_num][0] = '\0';
            }
        } else if (col < MAX_LINE_LENGTH - 1) {
            editor->lines[line_num][col++] = *ptr;
        }
        ptr++;
    }

    if (line_num < MAX_LINES) {
        editor->lines[line_num][col] = '\0';
        editor->line_count = line_num + 1;
    }
}

void editor_insert_char(TextEditor* editor, char c) {
    if (!editor || editor->cursor_line >= MAX_LINES) return;

    char* line = editor->lines[editor->cursor_line];
    size_t len = strlen(line);

    if (len < MAX_LINE_LENGTH - 1) {
        memmove(&line[editor->cursor_col + 1], &line[editor->cursor_col], len - editor->cursor_col + 1);
        line[editor->cursor_col] = c;
        editor->cursor_col++;
    }
}

void editor_backspace(TextEditor* editor) {
    if (!editor || editor->cursor_col == 0) return;

    char* line = editor->lines[editor->cursor_line];
    size_t len = strlen(line);

    if (editor->cursor_col <= len) {
        memmove(&line[editor->cursor_col - 1], &line[editor->cursor_col], len - editor->cursor_col + 1);
        editor->cursor_col--;
    }
}

void editor_delete_char(TextEditor* editor) {
    if (!editor) return;

    char* line = editor->lines[editor->cursor_line];
    size_t len = strlen(line);

    if (editor->cursor_col < len) {
        memmove(&line[editor->cursor_col], &line[editor->cursor_col + 1], len - editor->cursor_col);
    }
}

void editor_move_left(TextEditor* editor) {
    if (!editor || editor->cursor_col == 0) return;
    editor->cursor_col--;
    if (editor->cursor_col < editor->scroll_col) {
        editor->scroll_col = editor->cursor_col;
    }
}

void editor_move_right(TextEditor* editor) {
    if (!editor) return;
    char* line = editor->lines[editor->cursor_line];
    if (editor->cursor_col < (uint16_t)strlen(line)) {
        editor->cursor_col++;
    }
}

void editor_move_up(TextEditor* editor) {
    if (!editor || editor->cursor_line == 0) return;
    editor->cursor_line--;
    if (editor->cursor_line < editor->scroll_line) {
        editor->scroll_line = editor->cursor_line;
    }
}

void editor_move_down(TextEditor* editor) {
    if (!editor || editor->cursor_line >= editor->line_count - 1) return;
    editor->cursor_line++;
    if (editor->cursor_line >= editor->scroll_line + 6) {
        editor->scroll_line = editor->cursor_line - 5;
    }
}

void editor_insert_line(TextEditor* editor, const char* text) {
    if (!editor || editor->line_count >= MAX_LINES) return;

    for (size_t i = editor->line_count; i > editor->cursor_line; i--) {
        editor->lines[i] = editor->lines[i - 1];
    }

    editor->lines[editor->cursor_line] = malloc(MAX_LINE_LENGTH);
    if (text) {
        strncpy(editor->lines[editor->cursor_line], text, MAX_LINE_LENGTH - 1);
    } else {
        editor->lines[editor->cursor_line][0] = '\0';
    }

    editor->line_count++;
}

char* editor_get_text(TextEditor* editor) {
    if (!editor) return NULL;

    char* result = malloc(MAX_CODE_SIZE);
    size_t pos = 0;

    for (size_t i = 0; i < editor->line_count && pos < MAX_CODE_SIZE - 1; i++) {
        size_t line_len = strlen(editor->lines[i]);
        if (pos + line_len + 1 < MAX_CODE_SIZE) {
            strcpy(&result[pos], editor->lines[i]);
            pos += line_len;
            if (i < editor->line_count - 1) {
                result[pos++] = '\n';
            }
        }
    }

    result[pos] = '\0';
    return result;
}

#define MAX_CODE_SIZE 8192
