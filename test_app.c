#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINES 16
#define MAX_LINE_LEN 64

typedef enum {
    ModeEditor,
    ModeMenu,
} AppMode;

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    uint16_t line_count;
    uint16_t cursor_line;
    uint16_t cursor_col;
} TextEditor;

typedef struct {
    TextEditor editor;
    AppMode mode;
    uint16_t menu_index;
    bool running;
} AppState;

static void editor_init(TextEditor* ed) {
    ed->line_count = 1;
    ed->cursor_line = 0;
    ed->cursor_col = 0;
    memset(ed->lines, 0, sizeof(ed->lines));
}

static void editor_move_up(TextEditor* ed) {
    if (ed->cursor_line > 0) ed->cursor_line--;
}

static void editor_move_down(TextEditor* ed) {
    if (ed->cursor_line < ed->line_count - 1) ed->cursor_line++;
}

static void editor_move_left(TextEditor* ed) {
    if (ed->cursor_col > 0) ed->cursor_col--;
}

static void editor_move_right(TextEditor* ed) {
    if (ed->cursor_col < MAX_LINE_LEN - 1) ed->cursor_col++;
}

static void editor_new_line(TextEditor* ed) {
    if (ed->line_count < MAX_LINES) {
        ed->line_count++;
        ed->cursor_line++;
        ed->cursor_col = 0;
    }
}

static void editor_insert_code(TextEditor* ed, const char* code) {
    if (ed->line_count >= MAX_LINES) return;
    ed->line_count++;
    ed->cursor_line++;
    ed->cursor_col = 0;
    int len = strlen(code);
    if (len > MAX_LINE_LEN - 1) len = MAX_LINE_LEN - 1;
    for (int i = 0; i < len; i++) {
        ed->lines[ed->cursor_line][i] = code[i];
    }
    ed->lines[ed->cursor_line][len] = 0;
}

static void editor_draw(Canvas* canvas, AppState* state) {
    TextEditor* ed = &state->editor;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "C++ IDE");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    int max_lines = 5;
    for (int i = 0; i < max_lines && i < (int)ed->line_count; i++) {
        char num[4];
        num[0] = '0' + (i + 1) / 10;
        num[1] = '0' + (i + 1) % 10;
        num[2] = ':';
        num[3] = 0;
        canvas_draw_str(canvas, 0, 18 + i * 10, num);
        canvas_draw_str(canvas, 16, 18 + i * 10, ed->lines[i]);

        if (ed->cursor_line == (uint16_t)i) {
            int x = 16 + ed->cursor_col * 4;
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, x, 18 + i * 10 - 8, 1, 10);
            canvas_set_color(canvas, ColorBlack);
        }
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[OK] Menu");
}

static void menu_draw(Canvas* canvas, AppState* state) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "Menu");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    const char* items[] = {"> New Line", "  Insert Code", "  Clear", "  Exit"};
    for (int i = 0; i < 4; i++) {
        canvas_draw_str(canvas, 5, 20 + i * 10, items[i]);
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
                state->menu_index = 0;
                break;
            case InputKeyBack:
                break;
            default:
                break;
        }
    } else if (state->mode == ModeMenu) {
        switch (event->key) {
            case InputKeyUp:
                if (state->menu_index > 0) state->menu_index--;
                break;
            case InputKeyDown:
                if (state->menu_index < 3) state->menu_index++;
                break;
            case InputKeyOk:
                if (state->menu_index == 0) {
                    editor_new_line(&state->editor);
                } else if (state->menu_index == 1) {
                    editor_insert_code(&state->editor, "cout << endl;");
                } else if (state->menu_index == 2) {
                    editor_init(&state->editor);
                } else if (state->menu_index == 3) {
                    state->running = false;
                }
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
    } else {
        menu_draw(canvas, state);
    }
}

int32_t test_app_main(void* p) {
    UNUSED(p);

    AppState* state = malloc(sizeof(AppState));
    editor_init(&state->editor);
    state->mode = ModeEditor;
    state->menu_index = 0;
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
