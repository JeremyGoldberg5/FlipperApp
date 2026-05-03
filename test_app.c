#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINES 16
#define MAX_COL 64

typedef struct {
    char lines[MAX_LINES][MAX_COL];
    uint16_t line_count;
    uint16_t cursor_line;
    uint16_t cursor_col;
    bool running;
} AppState;

static void draw_callback(Canvas* canvas, void* ctx) {
    AppState* state = (AppState*)ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "C++ IDE");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    int max_shown = 5;
    for (int i = 0; i < max_shown && i < (int)state->line_count; i++) {
        int y = 18 + i * 10;
        char num[3];
        num[0] = '0' + (i + 1) / 10;
        num[1] = '0' + (i + 1) % 10;
        num[2] = 0;
        canvas_draw_str(canvas, 0, y, num);
        canvas_draw_str(canvas, 12, y, state->lines[i]);

        if ((uint16_t)i == state->cursor_line) {
            int x = 12 + state->cursor_col * 4;
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, x, y - 8, 1, 10);
            canvas_set_color(canvas, ColorBlack);
        }
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    char pos[24];
    snprintf(pos, sizeof(pos), "L:%u C:%u", state->cursor_line + 1, state->cursor_col);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, pos);
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
        case InputKeyOk:
            if (state->line_count < MAX_LINES) {
                state->line_count++;
                state->cursor_line++;
                state->cursor_col = 0;
            }
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
    memset(state->lines, 0, sizeof(state->lines));
    state->line_count = 1;
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
