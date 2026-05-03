#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINES 16

typedef enum {
    ModeEditor,
    ModeMenu,
} AppMode;

typedef struct {
    char lines[MAX_LINES][64];
    uint16_t line_count;
    uint16_t cursor_line;
    uint16_t cursor_col;
    AppMode mode;
    uint16_t menu_idx;
    bool running;
} AppState;

static void draw_callback(Canvas* canvas, void* ctx) {
    AppState* state = (AppState*)ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);

    if (state->mode == ModeEditor) {
        canvas_draw_str(canvas, 0, 8, "Editor");
        canvas_draw_line(canvas, 0, 10, 128, 10);

        int max_show = 4;
        for (int i = 0; i < max_show && i < (int)state->line_count; i++) {
            canvas_draw_str(canvas, 0, 18 + i * 10, state->lines[i]);
        }

        char info[32];
        snprintf(info, sizeof(info), "L:%u C:%u", state->cursor_line + 1, state->cursor_col);
        canvas_draw_str(canvas, 0, 62, info);
    } else {
        canvas_draw_str(canvas, 0, 8, "Menu");
        canvas_draw_line(canvas, 0, 10, 128, 10);

        canvas_draw_str(canvas, 5, 20, state->menu_idx == 0 ? "> New" : "  New");
        canvas_draw_str(canvas, 5, 30, state->menu_idx == 1 ? "> Code" : "  Code");
        canvas_draw_str(canvas, 5, 40, state->menu_idx == 2 ? "> Clear" : "  Clear");
        canvas_draw_str(canvas, 5, 50, state->menu_idx == 3 ? "> Exit" : "  Exit");

        canvas_draw_str(canvas, 0, 62, "UP/DOWN OK");
    }
}

static void input_callback(InputEvent* event, void* ctx) {
    AppState* state = (AppState*)ctx;

    if (event->type != InputTypePress) return;

    if (state->mode == ModeEditor) {
        switch (event->key) {
            case InputKeyUp:
                if (state->cursor_line > 0) state->cursor_line--;
                break;
            case InputKeyDown:
                if (state->cursor_line < state->line_count - 1) state->cursor_line++;
                break;
            case InputKeyLeft:
                if (state->cursor_col > 0) state->cursor_col--;
                break;
            case InputKeyRight:
                if (state->cursor_col < 63) state->cursor_col++;
                break;
            case InputKeyOk:
                state->mode = ModeMenu;
                state->menu_idx = 0;
                break;
            case InputKeyBack:
                if (state->cursor_col > 0) state->cursor_col--;
                break;
            default:
                break;
        }
    } else {
        switch (event->key) {
            case InputKeyUp:
                if (state->menu_idx > 0) state->menu_idx--;
                break;
            case InputKeyDown:
                if (state->menu_idx < 3) state->menu_idx++;
                break;
            case InputKeyOk:
                if (state->menu_idx == 0) {
                    if (state->line_count < MAX_LINES) {
                        state->line_count++;
                        state->cursor_line = state->line_count - 1;
                        state->cursor_col = 0;
                    }
                } else if (state->menu_idx == 1) {
                    if (state->line_count < MAX_LINES) {
                        const char* code = "cout << x;";
                        int len = strlen(code);
                        for (int i = 0; i < len && i < 63; i++) {
                            state->lines[state->line_count][i] = code[i];
                        }
                        state->lines[state->line_count][len < 63 ? len : 63] = 0;
                        state->line_count++;
                        state->cursor_line = state->line_count - 1;
                        state->cursor_col = 0;
                    }
                } else if (state->menu_idx == 2) {
                    memset(state->lines, 0, sizeof(state->lines));
                    state->line_count = 1;
                    state->cursor_line = 0;
                    state->cursor_col = 0;
                } else if (state->menu_idx == 3) {
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

int32_t test_app_main(void* p) {
    UNUSED(p);

    AppState* state = malloc(sizeof(AppState));
    memset(state->lines, 0, sizeof(state->lines));
    state->line_count = 1;
    state->cursor_line = 0;
    state->cursor_col = 0;
    state->mode = ModeEditor;
    state->menu_idx = 0;
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
