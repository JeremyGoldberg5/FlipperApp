#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINES 16
#define MAX_LINE_LENGTH 64

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    uint16_t line_count;
    uint16_t cursor_line;
    uint16_t cursor_col;
} SimpleEditor;

typedef enum {
    AppStateEditor,
    AppStateMenu,
} AppState;

typedef struct {
    SimpleEditor editor;
    AppState state;
    bool running;
} AppContext;

static void editor_init(SimpleEditor* ed) {
    ed->line_count = 1;
    ed->cursor_line = 0;
    ed->cursor_col = 0;
    memset(ed->lines, 0, sizeof(ed->lines));
    strcpy(ed->lines[0], "// Hello Flipper!");
}

static void editor_insert_char(SimpleEditor* ed, char c) {
    if (ed->line_count == 0) return;
    char* line = ed->lines[ed->cursor_line];
    size_t len = strlen(line);
    if (len >= MAX_LINE_LENGTH - 1) return;

    for (size_t i = len; i > ed->cursor_col; i--) {
        line[i] = line[i-1];
    }
    line[ed->cursor_col] = c;
    line[len+1] = '\0';
    ed->cursor_col++;
}

static void editor_backspace(SimpleEditor* ed) {
    if (ed->cursor_col == 0) return;
    char* line = ed->lines[ed->cursor_line];
    size_t len = strlen(line);

    for (size_t i = ed->cursor_col - 1; i < len; i++) {
        line[i] = line[i+1];
    }
    ed->cursor_col--;
}

static void editor_move_left(SimpleEditor* ed) {
    if (ed->cursor_col > 0) ed->cursor_col--;
}

static void editor_move_right(SimpleEditor* ed) {
    char* line = ed->lines[ed->cursor_line];
    if (ed->cursor_col < (uint16_t)strlen(line)) ed->cursor_col++;
}

static void editor_move_up(SimpleEditor* ed) {
    if (ed->cursor_line > 0) ed->cursor_line--;
    char* line = ed->lines[ed->cursor_line];
    if (ed->cursor_col > (uint16_t)strlen(line)) {
        ed->cursor_col = strlen(line);
    }
}

static void editor_move_down(SimpleEditor* ed) {
    if (ed->cursor_line < ed->line_count - 1) ed->cursor_line++;
    char* line = ed->lines[ed->cursor_line];
    if (ed->cursor_col > (uint16_t)strlen(line)) {
        ed->cursor_col = strlen(line);
    }
}

static void editor_insert_line(SimpleEditor* ed) {
    if (ed->line_count >= MAX_LINES - 1) return;
    ed->line_count++;
    ed->cursor_line++;
    ed->cursor_col = 0;
    memset(ed->lines[ed->cursor_line], 0, MAX_LINE_LENGTH);
}

static void editor_draw_callback(Canvas* canvas, void* ctx) {
    AppContext* context = (AppContext*)ctx;
    SimpleEditor* editor = &context->editor;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "C++ IDE");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    uint16_t max_lines = 5;
    for (uint16_t i = 0; i < max_lines && i < editor->line_count; i++) {
        char line_num[4];
        snprintf(line_num, sizeof(line_num), "%u:", i + 1);
        canvas_draw_str(canvas, 0, 18 + i * 10, line_num);

        char* line = editor->lines[i];
        canvas_draw_str(canvas, 12, 18 + i * 10, line);

        if (editor->cursor_line == i) {
            uint16_t cursor_x = 12 + editor->cursor_col * 5;
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, cursor_x, 18 + i * 10 - 8, 1, 10);
        }
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[OK] Menu [BACK] Del");
}

static void menu_draw_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "Menu");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    canvas_draw_str(canvas, 5, 25, "> New Line");
    canvas_draw_str(canvas, 5, 35, "  Clear");
    canvas_draw_str(canvas, 5, 45, "  Exit");

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[OK/BACK] Select");
}

static void input_callback(InputEvent* event, void* ctx) {
    AppContext* context = (AppContext*)ctx;

    if (event->type != InputTypePress) return;

    if (context->state == AppStateEditor) {
        switch (event->key) {
            case InputKeyUp:
                editor_move_up(&context->editor);
                break;
            case InputKeyDown:
                editor_move_down(&context->editor);
                break;
            case InputKeyLeft:
                editor_move_left(&context->editor);
                break;
            case InputKeyRight:
                editor_move_right(&context->editor);
                break;
            case InputKeyOk:
                context->state = AppStateMenu;
                break;
            case InputKeyBack:
                editor_backspace(&context->editor);
                break;
            default:
                break;
        }
    } else if (context->state == AppStateMenu) {
        switch (event->key) {
            case InputKeyUp:
                break;
            case InputKeyDown:
                break;
            case InputKeyOk:
                editor_insert_line(&context->editor);
                context->state = AppStateEditor;
                break;
            case InputKeyBack:
                context->state = AppStateEditor;
                break;
            default:
                break;
        }
    }
}

int32_t test_app_main(void* p) {
    UNUSED(p);

    AppContext* context = malloc(sizeof(AppContext));
    editor_init(&context->editor);
    context->state = AppStateEditor;
    context->running = true;

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, editor_draw_callback, context);
    view_port_input_callback_set(view_port, input_callback, context);

    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    while(context->running) {
        if (context->state == AppStateMenu) {
            view_port_draw_callback_set(view_port, menu_draw_callback, context);
        } else {
            view_port_draw_callback_set(view_port, editor_draw_callback, context);
        }

        view_port_update(view_port);
        furi_delay_ms(10);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    free(context);

    return 0;
}
