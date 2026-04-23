#pragma once

#include <gui/gui.h>

#define MAX_LINES 256
#define MAX_LINE_LENGTH 256

typedef struct {
    char* lines[MAX_LINES];
    uint16_t line_count;
    uint16_t cursor_line;
    uint16_t cursor_col;
    uint16_t scroll_line;
    uint16_t scroll_col;
} TextEditor;

TextEditor* editor_alloc(void);
void editor_free(TextEditor* editor);
void editor_clear(TextEditor* editor);
void editor_load_text(TextEditor* editor, const char* text);
void editor_insert_char(TextEditor* editor, char c);
void editor_backspace(TextEditor* editor);
void editor_delete_char(TextEditor* editor);
void editor_move_left(TextEditor* editor);
void editor_move_right(TextEditor* editor);
void editor_move_up(TextEditor* editor);
void editor_move_down(TextEditor* editor);
void editor_insert_line(TextEditor* editor, const char* text);
char* editor_get_text(TextEditor* editor);
