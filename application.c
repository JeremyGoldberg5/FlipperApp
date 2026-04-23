#include "application.h"
#include "editor/editor.h"
#include "editor/snippets.h"
#include "interpreter/lexer.h"
#include "interpreter/parser.h"
#include "interpreter/executor.h"
#include <furi.h>
#include <gui/elements.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    AppStateEditor,
    AppStateMenu,
    AppStateSnippets,
    AppStateOutput,
} AppState;

typedef struct {
    TextEditor* editor;
    AppState state;
    size_t selected_snippet;
    Executor* executor;
    FuriMutex* state_mutex;
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
    canvas_draw_str(canvas, 5, 45, "  Run Code");
    canvas_draw_str(canvas, 5, 55, "  Clear");

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

static void output_draw_callback(Canvas* canvas, void* ctx) {
    AppContext* context = (AppContext*)ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 0, 8, "Output");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    const char* output = executor_get_output(context->executor);
    if (!output || strlen(output) == 0) {
        canvas_draw_str(canvas, 0, 25, "No output");
    } else {
        const char* ptr = output;
        int y = 18;
        char line[50];
        int line_pos = 0;

        while (*ptr && y < 60) {
            if (*ptr == '\n' || line_pos >= 49) {
                line[line_pos] = '\0';
                canvas_draw_str(canvas, 0, y, line);
                y += 8;
                line_pos = 0;
                if (*ptr == '\n') ptr++;
            } else {
                line[line_pos++] = *ptr++;
            }
        }
        if (line_pos > 0) {
            line[line_pos] = '\0';
            canvas_draw_str(canvas, 0, y, line);
        }
    }

    canvas_draw_line(canvas, 0, 62, 128, 62);
    canvas_draw_str_aligned(canvas, 64, 64, AlignCenter, AlignBottom, "[BACK] Return");
}

static void input_callback(InputEvent event, void* ctx) {
    AppContext* context = (AppContext*)ctx;

    if (event.type != InputTypePress) return;

    furi_mutex_acquire(context->state_mutex, FuriWaitForever);

    if (context->state == AppStateEditor) {
        switch (event.key) {
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
        switch (event.key) {
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
        switch (event.key) {
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
    } else if (context->state == AppStateOutput) {
        switch (event.key) {
            case InputKeyOk:
            case InputKeyBack:
                context->state = AppStateEditor;
                break;
            default:
                break;
        }
    }

    furi_mutex_release(context->state_mutex);
}

CppIdeApp* cpp_ide_app_alloc(void) {
    CppIdeApp* app = malloc(sizeof(CppIdeApp));
    memset(app, 0, sizeof(CppIdeApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    strcpy(app->current_file, "program.cpp");

    return app;
}

void cpp_ide_app_free(CppIdeApp* app) {
    if (!app) return;
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_DIALOGS);
    free(app);
}

int32_t cpp_ide_app_main(void* p) {
    UNUSED(p);

    AppContext* context = malloc(sizeof(AppContext));
    context->editor = editor_alloc();
    context->executor = executor_create();
    context->state = AppStateEditor;
    context->selected_snippet = 0;
    context->state_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    CppIdeApp* app = cpp_ide_app_alloc();

    ViewPort* view_port = view_port_alloc();
    InputQueue* input_queue = furi_record_open(RECORD_GUI);

    view_port_draw_callback_set(view_port, editor_draw_callback, context);
    view_port_input_callback_set(view_port, input_callback, context);

    gui_add_view_port(app->gui, view_port, GuiLayerFullscreen);

    while (1) {
        furi_mutex_acquire(context->state_mutex, FuriWaitForever);

        AppState current_state = context->state;

        if (current_state == AppStateOutput) {
            view_port_draw_callback_set(view_port, output_draw_callback, context);
        } else if (current_state == AppStateMenu) {
            view_port_draw_callback_set(view_port, menu_draw_callback, context);
        } else if (current_state == AppStateSnippets) {
            view_port_draw_callback_set(view_port, snippets_draw_callback, context);
        } else {
            view_port_draw_callback_set(view_port, editor_draw_callback, context);
        }

        view_port_update(view_port);

        furi_mutex_release(context->state_mutex);
        furi_delay_ms(50);
    }

    gui_remove_view_port(app->gui, view_port);
    view_port_free(view_port);

    executor_free(context->executor);
    editor_free(context->editor);
    furi_mutex_free(context->state_mutex);
    free(context);
    cpp_ide_app_free(app);

    return 0;
}
