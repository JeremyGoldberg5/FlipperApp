#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "editor/editor.h"
#include "editor/snippets.h"

typedef enum {
    AppStateEditor,
    AppStateMenu,
    AppStateSnippets,
} AppState;

typedef struct {
    TextEditor* editor;
    AppState state;
    size_t selected_snippet;
} AppContext;

static void editor_draw_callback(Canvas* canvas, void* ctx) {
    AppContext* context = (AppContext*)ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "C++ IDE - Editor");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    TextEditor* editor = context->editor;
    uint16_t start_line = editor->scroll_line;
    uint16_t max_lines = 5;

    for (uint16_t i = 0; i < max_lines && start_line + i < editor->line_count; i++) {
        char line_num[4];
        snprintf(line_num, sizeof(line_num), "%2u", start_line + i + 1);
        canvas_draw_str(canvas, 0, 18 + i * 10, line_num);

        char displayed_text[50];
        const char* line = editor->lines[start_line + i];
        size_t disp_len = strlen(line) > 40 ? 40 : strlen(line);
        if (disp_len > 0) {
            strncpy(displayed_text, line, disp_len);
        }
        displayed_text[disp_len] = '\0';

        canvas_draw_str(canvas, 16, 18 + i * 10, displayed_text);

        if (editor->cursor_line == start_line + i && editor->cursor_col <= (uint16_t)strlen(line)) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 16 + editor->cursor_col * 5, 18 + i * 10 - 8, 1, 10);
        }
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[OK] Menu");
}

static void menu_draw_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "Menu");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    canvas_draw_str(canvas, 5, 25, "> New Code");
    canvas_draw_str(canvas, 5, 35, "  Snippets");
    canvas_draw_str(canvas, 5, 45, "  Clear");
    canvas_draw_str(canvas, 5, 55, "  Exit");

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[OK/BACK] Select");
}

static void snippets_draw_callback(Canvas* canvas, void* ctx) {
    AppContext* context = (AppContext*)ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "Code Snippets");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    uint16_t start = context->selected_snippet > 2 ? context->selected_snippet - 2 : 0;
    uint16_t max_shown = 5;

    for (size_t i = 0; i < max_shown && start + i < snippets_count; i++) {
        if (start + i == context->selected_snippet) {
            canvas_draw_str(canvas, 2, 20 + i * 8, ">");
        }
        canvas_draw_str(canvas, 12, 20 + i * 8, snippets[start + i].name);
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[UP/DOWN/OK/BACK]");
}

static void input_callback(InputEvent* event, void* ctx) {
    AppContext* context = (AppContext*)ctx;

    if (event->type != InputTypePress) return;

    if (context->state == AppStateEditor) {
        switch (event->key) {
            case InputKeyUp:
                editor_move_up(context->editor);
                break;
            case InputKeyDown:
                editor_move_down(context->editor);
                break;
            case InputKeyLeft:
                editor_move_left(context->editor);
                break;
            case InputKeyRight:
                editor_move_right(context->editor);
                break;
            case InputKeyOk:
                context->state = AppStateMenu;
                break;
            case InputKeyBack:
                editor_backspace(context->editor);
                break;
            default:
                break;
        }
    } else if (context->state == AppStateMenu) {
        switch (event->key) {
            case InputKeyOk:
                context->state = AppStateSnippets;
                break;
            case InputKeyBack:
                context->state = AppStateEditor;
                break;
            default:
                break;
        }
    } else if (context->state == AppStateSnippets) {
        switch (event->key) {
            case InputKeyUp:
                if (context->selected_snippet > 0) context->selected_snippet--;
                break;
            case InputKeyDown:
                if (context->selected_snippet < snippets_count - 1) context->selected_snippet++;
                break;
            case InputKeyOk: {
                const char* code = snippets[context->selected_snippet].code;
                editor_insert_line(context->editor, code);
                context->state = AppStateEditor;
                break;
            }
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
    context->editor = editor_alloc();
    context->state = AppStateEditor;
    context->selected_snippet = 0;

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, editor_draw_callback, context);
    view_port_input_callback_set(view_port, input_callback, context);

    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    bool running = true;
    while(running) {
        AppState current_state = context->state;

        if (current_state == AppStateMenu) {
            view_port_draw_callback_set(view_port, menu_draw_callback, context);
        } else if (current_state == AppStateSnippets) {
            view_port_draw_callback_set(view_port, snippets_draw_callback, context);
        } else {
            view_port_draw_callback_set(view_port, editor_draw_callback, context);
        }

        view_port_update(view_port);

        furi_delay_ms(10);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    editor_free(context->editor);
    free(context);

    return 0;
}
