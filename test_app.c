#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINES 24
#define MAX_COL 80
#define NUM_SNIPPETS 6

typedef enum {
    ModeEditor,
    ModeMenu,
    ModeSnippets,
} AppMode;

typedef struct {
    char lines[MAX_LINES][MAX_COL];
    uint16_t line_count;
    uint16_t cursor_line;
    uint16_t cursor_col;
    uint16_t scroll_line;
} Editor;

typedef struct {
    Editor editor;
    AppMode mode;
    uint16_t menu_idx;
    uint16_t snippet_idx;
    bool running;
} AppState;

static const char* snippet_names[] = {"cout", "for", "if", "while", "class", "int main"};
static const char* snippet_code[] = {
    "cout << x << endl;",
    "for(int i=0; i<10; i++) {",
    "if(x > 0) {",
    "while(true) {",
    "class MyClass {",
    "int main() {"
};

static void editor_insert_snippet(Editor* ed, const char* code) {
    if (ed->line_count >= MAX_LINES) return;
    int len = strlen(code);
    if (len > MAX_COL - 1) len = MAX_COL - 1;
    for (int i = 0; i < len; i++) {
        ed->lines[ed->line_count][i] = code[i];
    }
    ed->lines[ed->line_count][len] = 0;
    ed->line_count++;
    ed->cursor_line = ed->line_count - 1;
    ed->cursor_col = 0;
}

static void editor_new_line(Editor* ed) {
    if (ed->line_count < MAX_LINES) {
        ed->line_count++;
        ed->cursor_line++;
        ed->cursor_col = 0;
    }
}

static void editor_backspace(Editor* ed) {
    if (ed->cursor_col == 0) return;
    char* line = ed->lines[ed->cursor_line];
    int len = strlen(line);
    for (int i = ed->cursor_col - 1; i < len; i++) {
        line[i] = line[i + 1];
    }
    ed->cursor_col--;
}

static void draw_editor(Canvas* canvas, AppState* state) {
    Editor* ed = &state->editor;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "C++ IDE - Editor");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    int max_show = 5;
    for (int i = 0; i < max_show && ed->scroll_line + i < ed->line_count; i++) {
        int y = 18 + i * 10;
        char num[3];
        num[0] = '0' + (ed->scroll_line + i + 1) / 10;
        num[1] = '0' + (ed->scroll_line + i + 1) % 10;
        num[2] = 0;
        canvas_draw_str(canvas, 0, y, num);
        canvas_draw_str(canvas, 12, y, ed->lines[ed->scroll_line + i]);

        if (ed->cursor_line == ed->scroll_line + i) {
            int x = 12 + ed->cursor_col * 4;
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, x, y - 8, 1, 10);
            canvas_set_color(canvas, ColorBlack);
        }
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    char info[24];
    snprintf(info, sizeof(info), "L:%u C:%u", ed->cursor_line + 1, ed->cursor_col);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, info);
}

static void draw_menu(Canvas* canvas, AppState* state) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "Menu");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    const char* items[] = {"> Insert Code", "  New Line", "  Clear All", "  Exit"};
    for (int i = 0; i < 4; i++) {
        canvas_draw_str(canvas, 5, 20 + i * 10, items[i]);
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[UP/DOWN/OK/BACK]");
}

static void draw_snippets(Canvas* canvas, AppState* state) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "Code Snippets");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    for (int i = 0; i < NUM_SNIPPETS; i++) {
        char line[32];
        char prefix = (i == (int)state->snippet_idx) ? '>' : ' ';
        snprintf(line, sizeof(line), "%c %s", prefix, snippet_names[i]);
        canvas_draw_str(canvas, 5, 20 + i * 8, line);
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[UP/DOWN/OK/BACK]");
}

static void draw_callback(Canvas* canvas, void* ctx) {
    AppState* state = (AppState*)ctx;

    if (state->mode == ModeEditor) {
        draw_editor(canvas, state);
    } else if (state->mode == ModeMenu) {
        draw_menu(canvas, state);
    } else if (state->mode == ModeSnippets) {
        draw_snippets(canvas, state);
    }
}

static void input_callback(InputEvent* event, void* ctx) {
    AppState* state = (AppState*)ctx;

    if (event->type != InputTypePress) return;

    if (state->mode == ModeEditor) {
        switch (event->key) {
            case InputKeyUp:
                if (state->editor.cursor_line > 0) {
                    state->editor.cursor_line--;
                    if (state->editor.cursor_line < state->editor.scroll_line) {
                        state->editor.scroll_line = state->editor.cursor_line;
                    }
                }
                break;
            case InputKeyDown:
                if (state->editor.cursor_line < state->editor.line_count - 1) {
                    state->editor.cursor_line++;
                    if (state->editor.cursor_line >= state->editor.scroll_line + 5) {
                        state->editor.scroll_line = state->editor.cursor_line - 4;
                    }
                }
                break;
            case InputKeyLeft:
                if (state->editor.cursor_col > 0) state->editor.cursor_col--;
                break;
            case InputKeyRight:
                if (state->editor.cursor_col < MAX_COL - 1) state->editor.cursor_col++;
                break;
            case InputKeyOk:
                state->mode = ModeMenu;
                state->menu_idx = 0;
                break;
            case InputKeyBack:
                editor_backspace(&state->editor);
                break;
            default:
                break;
        }
    } else if (state->mode == ModeMenu) {
        switch (event->key) {
            case InputKeyUp:
                if (state->menu_idx > 0) state->menu_idx--;
                break;
            case InputKeyDown:
                if (state->menu_idx < 3) state->menu_idx++;
                break;
            case InputKeyOk:
                if (state->menu_idx == 0) {
                    state->mode = ModeSnippets;
                    state->snippet_idx = 0;
                } else if (state->menu_idx == 1) {
                    editor_new_line(&state->editor);
                    state->mode = ModeEditor;
                } else if (state->menu_idx == 2) {
                    memset(state->editor.lines, 0, sizeof(state->editor.lines));
                    state->editor.line_count = 1;
                    state->editor.cursor_line = 0;
                    state->editor.cursor_col = 0;
                    state->editor.scroll_line = 0;
                    state->mode = ModeEditor;
                } else if (state->menu_idx == 3) {
                    state->running = false;
                }
                break;
            case InputKeyBack:
                state->mode = ModeEditor;
                break;
            default:
                break;
        }
    } else if (state->mode == ModeSnippets) {
        switch (event->key) {
            case InputKeyUp:
                if (state->snippet_idx > 0) state->snippet_idx--;
                break;
            case InputKeyDown:
                if (state->snippet_idx < NUM_SNIPPETS - 1) state->snippet_idx++;
                break;
            case InputKeyOk:
                editor_insert_snippet(&state->editor, snippet_code[state->snippet_idx]);
                state->mode = ModeEditor;
                break;
            case InputKeyBack:
                state->mode = ModeEditor;
                break;
            default:
                break;
        }
    }
}

int32_t test_app_main(void* p) {
    UNUSED(p);

    AppState* state = malloc(sizeof(AppState));
    memset(state->editor.lines, 0, sizeof(state->editor.lines));
    state->editor.line_count = 1;
    state->editor.cursor_line = 0;
    state->editor.cursor_col = 0;
    state->editor.scroll_line = 0;
    state->mode = ModeEditor;
    state->menu_idx = 0;
    state->snippet_idx = 0;
    state->running = true;

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, draw_callback, state);
    view_port_input_callback_set(view_port, input_callback, state);

    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    while (state->running) {
        view_port_update(view_port);
        furi_delay_ms(10);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    free(state);

    return 0;
}
