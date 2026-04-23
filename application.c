#include "application.h"
#include "editor/editor.h"
#include "editor/snippets.h"
#include "interpreter/lexer.h"
#include "interpreter/parser.h"
#include "interpreter/executor.h"
#include <furi.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    AppStateMenu,
    AppStateEditor,
    AppStateSnippets,
    AppStateOutput,
} AppState;

typedef struct {
    CppIdeApp* app;
    TextEditor* editor;
    AppState state;
    size_t selected_snippet;
    Executor* executor;
} AppContext;

static void editor_draw_callback(Canvas* canvas, void* ctx) {
    AppContext* context = (AppContext*)ctx;

    canvas_clear(canvas);
    canvas_draw_str(canvas, 0, 10, "C++ IDE - Editor");
    canvas_draw_line(canvas, 0, 12, 128, 12);

    TextEditor* editor = context->editor;
    uint16_t start_line = editor->scroll_line;
    uint16_t max_lines = 6;

    for (uint16_t i = 0; i < max_lines && start_line + i < editor->line_count; i++) {
        char line_num[4];
        snprintf(line_num, sizeof(line_num), "%2d", start_line + i + 1);
        canvas_draw_str(canvas, 0, 20 + i * 8, line_num);

        char displayed_text[50];
        const char* line = editor->lines[start_line + i];
        size_t disp_len = strlen(line) > 40 ? 40 : strlen(line);
        strncpy(displayed_text, line, disp_len);
        displayed_text[disp_len] = '\0';

        canvas_draw_str(canvas, 18, 20 + i * 8, displayed_text);

        if (editor->cursor_line == start_line + i) {
            canvas_draw_str(canvas, 18 + editor->cursor_col * 6, 20 + i * 8 + 1, "_");
        }
    }

    canvas_draw_str(canvas, 0, 62, "[UP/DOWN] Move [LEFT/RIGHT] Edit [OK] Menu");
}

static void output_draw_callback(Canvas* canvas, void* ctx) {
    AppContext* context = (AppContext*)ctx;

    canvas_clear(canvas);
    canvas_draw_str(canvas, 0, 10, "Output");
    canvas_draw_line(canvas, 0, 12, 128, 12);

    const char* output = executor_get_output(context->executor);
    if (!output || strlen(output) == 0) {
        canvas_draw_str(canvas, 0, 25, "No output");
    } else {
        const char* ptr = output;
        int y = 20;
        char line[40];
        int line_pos = 0;

        while (*ptr && y < 60) {
            if (*ptr == '\n' || line_pos >= 39) {
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

    canvas_draw_str(canvas, 0, 62, "[OK] Back");
}

static void menu_draw_callback(Canvas* canvas, void* ctx) {
    AppContext* context = (AppContext*)ctx;

    canvas_clear(canvas);
    canvas_draw_str(canvas, 0, 10, "C++ IDE");
    canvas_draw_line(canvas, 0, 12, 128, 12);

    canvas_draw_str(canvas, 5, 25, "1. New Code");
    canvas_draw_str(canvas, 5, 35, "2. Load File");
    canvas_draw_str(canvas, 5, 45, "3. Save File");
    canvas_draw_str(canvas, 5, 55, "4. Run Code");
}

static void snippets_draw_callback(Canvas* canvas, void* ctx) {
    AppContext* context = (AppContext*)ctx;

    canvas_clear(canvas);
    canvas_draw_str(canvas, 0, 10, "Code Snippets");
    canvas_draw_line(canvas, 0, 12, 128, 12);

    uint16_t start = context->selected_snippet > 2 ? context->selected_snippet - 2 : 0;
    uint16_t max_shown = 5;

    for (size_t i = 0; i < max_shown && start + i < snippets_count; i++) {
        if (start + i == context->selected_snippet) {
            canvas_draw_str(canvas, 2, 20 + i * 8, ">");
        }
        canvas_draw_str(canvas, 10, 20 + i * 8, snippets[start + i].name);
    }

    canvas_draw_str(canvas, 0, 62, "[UP/DOWN] Select [OK] Insert [BACK] Exit");
}

static void handle_editor_input(AppContext* context, InputEvent event) {
    if (event.type != InputTypePress) return;

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
}

static void handle_menu_input(AppContext* context, InputEvent event) {
    if (event.type != InputTypePress) return;

    switch (event.key) {
        case InputKeyOk:
            context->state = AppStateEditor;
            break;
        case InputKeyBack:
            context->state = AppStateEditor;
            break;
        default:
            break;
    }
}

static void handle_snippets_input(AppContext* context, InputEvent event) {
    if (event.type != InputTypePress) return;

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
}

static void handle_output_input(AppContext* context, InputEvent event) {
    if (event.type != InputTypePress) return;

    switch (event.key) {
        case InputKeyOk:
        case InputKeyBack:
            context->state = AppStateEditor;
            break;
        default:
            break;
    }
}

static void input_callback(InputEvent event, void* ctx) {
    AppContext* context = (AppContext*)ctx;

    switch (context->state) {
        case AppStateEditor:
            handle_editor_input(context, event);
            break;
        case AppStateMenu:
            handle_menu_input(context, event);
            break;
        case AppStateSnippets:
            handle_snippets_input(context, event);
            break;
        case AppStateOutput:
            handle_output_input(context, event);
            break;
    }
}

CppIdeApp* cpp_ide_app_alloc(void) {
    CppIdeApp* app = malloc(sizeof(CppIdeApp));
    memset(app, 0, sizeof(CppIdeApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    app->menu = menu_alloc();
    app->text_input = text_input_alloc();
    app->popup = popup_alloc();
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeWindow);

    strcpy(app->current_file, "program.cpp");
    app->code_length = 0;
    app->output_length = 0;

    return app;
}

void cpp_ide_app_free(CppIdeApp* app) {
    if (!app) return;

    if (app->menu) menu_free(app->menu);
    if (app->text_input) text_input_free(app->text_input);
    if (app->popup) popup_free(app->popup);
    if (app->view_dispatcher) view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_DIALOGS);

    free(app);
}

int32_t cpp_ide_app_main(void* p) {
    UNUSED(p);

    AppContext context = {};
    context.app = cpp_ide_app_alloc();
    context.editor = editor_alloc();
    context.executor = executor_create();
    context.state = AppStateEditor;

    FuriThread* input_thread = furi_thread_alloc();
    furi_thread_set_callback(input_thread, (FuriThreadCallback)input_callback);
    furi_thread_set_context(input_thread, &context);

    while (1) {
        switch (context.state) {
            case AppStateEditor: {
                canvas_frame_set(NULL);
                editor_draw_callback(NULL, &context);
                break;
            }
            case AppStateMenu: {
                menu_draw_callback(NULL, &context);
                char* code = editor_get_text(context.editor);

                Parser parser = parser_create(code);
                ASTNode* ast = parser_parse(&parser);

                executor_execute(context.executor, ast);
                ast_node_free(ast);

                context.state = AppStateOutput;
                free(code);
                break;
            }
            case AppStateSnippets: {
                snippets_draw_callback(NULL, &context);
                break;
            }
            case AppStateOutput: {
                output_draw_callback(NULL, &context);
                break;
            }
        }

        furi_delay_ms(100);
    }

    executor_free(context.executor);
    editor_free(context.editor);
    cpp_ide_app_free(context.app);
    return 0;
}
