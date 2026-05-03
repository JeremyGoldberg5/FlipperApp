#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    uint16_t cursor_line;
    uint16_t cursor_col;
    bool running;
} AppState;

static void draw_callback(Canvas* canvas, void* ctx) {
    AppState* state = (AppState*)ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 10, 20, "C++ IDE");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 10, 35, "Line navigation");
    canvas_draw_str(canvas, 10, 45, "with arrow keys");

    char text[32];
    snprintf(text, sizeof(text), "Line: %u Col: %u", state->cursor_line, state->cursor_col);
    canvas_draw_str(canvas, 10, 55, text);
}

static void input_callback(InputEvent* event, void* ctx) {
    AppState* state = (AppState*)ctx;

    if (event->type != InputTypePress) return;

    switch (event->key) {
        case InputKeyUp:
            if (state->cursor_line > 0) state->cursor_line--;
            break;
        case InputKeyDown:
            state->cursor_line++;
            break;
        case InputKeyLeft:
            if (state->cursor_col > 0) state->cursor_col--;
            break;
        case InputKeyRight:
            state->cursor_col++;
            break;
        case InputKeyBack:
            state->running = false;
            break;
        default:
            break;
    }
}

int32_t test_app_main(void* p) {
    UNUSED(p);

    AppState* state = malloc(sizeof(AppState));
    state->cursor_line = 0;
    state->cursor_col = 0;
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
