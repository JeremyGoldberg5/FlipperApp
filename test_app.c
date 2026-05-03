#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINES 32
#define MAX_LINE_LEN 80
#define MAX_SNIPPETS 5

typedef enum {
    ModeEditor,
    ModeMenu,
    ModeSnippets,
} AppMode;

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    uint16_t line_count;
    uint16_t cursor_line;
    uint16_t cursor_col;
    uint16_t scroll_line;
} TextEditor;

typedef struct {
    const char* name;
    const char* code;
} CodeSnippet;

typedef struct {
    TextEditor editor;
    AppMode mode;
    uint16_t menu_selection;
    uint16_t snippet_selection;
    bool running;
} AppState;

static const CodeSnippet snippets[MAX_SNIPPETS] = {
    {"Hello World", "cout << \"Hello!\" << endl;"},
    {"For Loop", "for(int i=0; i<10; i++)"},
    {"If Statement", "if(x > 0) { }"},
    {"Function", "int add(int a, int b) {"},
    {"Array", "int arr[10];"},
};

static void editor_init(TextEditor* ed) {
    ed->line_count = 1;
    ed->cursor_line = 0;
    ed->cursor_col = 0;
    ed->scroll_line = 0;
    memset(ed->lines, 0, sizeof(ed->lines));
}

static void editor_insert_line(TextEditor* ed) {
    if (ed->line_count >= MAX_LINES) return;
    ed->line_count++;
    ed->cursor_line++;
    ed->cursor_col = 0;
    memset(ed->lines[ed->cursor_line], 0, MAX_LINE_LEN);
}

static void editor_insert_snippet(TextEditor* ed, const char* code) {
    if (ed->line_count >= MAX_LINES) return;
    ed->line_count++;
    ed->cursor_line++;
    ed->cursor_col = 0;
    size_t len = strlen(code);
    if (len >= MAX_LINE_LEN) len = MAX_LINE_LEN - 1;
    memcpy(ed->lines[ed->cursor_line], code, len);
    ed->lines[ed->cursor_line][len] = '\0';
}

static void editor_move_up(TextEditor* ed) {
    if (ed->cursor_line > 0) {
        ed->cursor_line--;
        if (ed->cursor_col > (uint16_t)strlen(ed->lines[ed->cursor_line])) {
            ed->cursor_col = strlen(ed->lines[ed->cursor_line]);
        }
    }
    if (ed->cursor_line < ed->scroll_line) {
        ed->scroll_line = ed->cursor_line;
    }
}

static void editor_move_down(TextEditor* ed) {
    if (ed->cursor_line < ed->line_count - 1) {
        ed->cursor_line++;
        if (ed->cursor_col > (uint16_t)strlen(ed->lines[ed->cursor_line])) {
            ed->cursor_col = strlen(ed->lines[ed->cursor_line]);
        }
    }
    if (ed->cursor_line >= ed->scroll_line + 5) {
        ed->scroll_line = ed->cursor_line - 4;
    }
}

static void editor_move_left(TextEditor* ed) {
    if (ed->cursor_col > 0) ed->cursor_col--;
}

static void editor_move_right(TextEditor* ed) {
    uint16_t len = strlen(ed->lines[ed->cursor_line]);
    if (ed->cursor_col < len) ed->cursor_col++;
}

static void editor_backspace(TextEditor* ed) {
    if (ed->cursor_col == 0) return;
    char* line = ed->lines[ed->cursor_line];
    uint16_t len = strlen(line);
    for (uint16_t i = ed->cursor_col - 1; i < len; i++) {
        line[i] = line[i + 1];
    }
    ed->cursor_col--;
}

static void editor_draw(Canvas* canvas, AppState* state) {
    TextEditor* ed = &state->editor;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "C++ IDE - Editor");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    uint16_t max_lines = 5;
    for (uint16_t i = 0; i < max_lines && ed->scroll_line + i < ed->line_count; i++) {
        char line_num[4];
        snprintf(line_num, sizeof(line_num), "%u:", ed->scroll_line + i + 1);
        canvas_draw_str(canvas, 0, 18 + i * 10, line_num);

        canvas_draw_str(canvas, 16, 18 + i * 10, ed->lines[ed->scroll_line + i]);

        if (ed->cursor_line == ed->scroll_line + i) {
            uint16_t x = 16 + ed->cursor_col * 5;
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, x, 18 + i * 10 - 8, 1, 10);
            canvas_set_color(canvas, ColorBlack);
        }
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[OK] Menu [BACK] Delete");
}

static void menu_draw(Canvas* canvas, AppState* state) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "Menu");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    const char* options[] = {"> New Line", "  Snippets", "  Clear", "  Exit"};
    for (int i = 0; i < 4; i++) {
        canvas_draw_str(canvas, 5, 20 + i * 10, options[i]);
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[UP/DOWN/OK/BACK]");
}

static void snippets_draw(Canvas* canvas, AppState* state) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "Code Snippets");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    for (int i = 0; i < MAX_SNIPPETS; i++) {
        char prefix = (i == (int)state->snippet_selection) ? '>' : ' ';
        char line[64];
        snprintf(line, sizeof(line), "%c %s", prefix, snippets[i].name);
        canvas_draw_str(canvas, 5, 20 + i * 8, line);
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[UP/DOWN/OK/BACK]");
}

static void input_callback(InputEvent* event, void* ctx) {
    AppState* state = (AppState*)ctx;

    if (event->type != InputTypePress) return;

    if (state->mode == ModeEditor) {
        switch (event->key) {
            case InputKeyUp:
                editor_move_up(&state->editor);
                break;
            case InputKeyDown:
                editor_move_down(&state->editor);
                break;
            case InputKeyLeft:
                editor_move_left(&state->editor);
                break;
            case InputKeyRight:
                editor_move_right(&state->editor);
                break;
            case InputKeyOk:
                state->mode = ModeMenu;
                state->menu_selection = 0;
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
                if (state->menu_selection > 0) state->menu_selection--;
                break;
            case InputKeyDown:
                if (state->menu_selection < 3) state->menu_selection++;
                break;
            case InputKeyOk:
                if (state->menu_selection == 0) {
                    editor_insert_line(&state->editor);
                    state->mode = ModeEditor;
                } else if (state->menu_selection == 1) {
                    state->mode = ModeSnippets;
                    state->snippet_selection = 0;
                } else if (state->menu_selection == 2) {
                    editor_init(&state->editor);
                    state->mode = ModeEditor;
                } else if (state->menu_selection == 3) {
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
                if (state->snippet_selection > 0) state->snippet_selection--;
                break;
            case InputKeyDown:
                if (state->snippet_selection < MAX_SNIPPETS - 1) state->snippet_selection++;
                break;
            case InputKeyOk:
                editor_insert_snippet(&state->editor, snippets[state->snippet_selection].code);
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

static void draw_callback(Canvas* canvas, void* ctx) {
    AppState* state = (AppState*)ctx;

    if (state->mode == ModeEditor) {
        editor_draw(canvas, state);
    } else if (state->mode == ModeMenu) {
        menu_draw(canvas, state);
    } else if (state->mode == ModeSnippets) {
        snippets_draw(canvas, state);
    }
}

int32_t test_app_main(void* p) {
    UNUSED(p);

    AppState* state = malloc(sizeof(AppState));
    editor_init(&state->editor);
    state->mode = ModeEditor;
    state->menu_selection = 0;
    state->snippet_selection = 0;
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
