#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>

typedef struct {
    FuriMessageQueue* queue;
} AppState;

static void draw_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 10, 20, "C++ IDE");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 10, 35, "Press BACK to exit");
}

static void input_callback(InputEvent* event, void* ctx) {
    AppState* state = (AppState*)ctx;
    furi_message_queue_put(state->queue, event, FuriWaitForever);
}

int32_t test_app_main(void* p) {
    UNUSED(p);

    AppState* state = malloc(sizeof(AppState));
    state->queue = furi_message_queue_alloc(32, sizeof(InputEvent));

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, draw_callback, state);
    view_port_input_callback_set(view_port, input_callback, state);

    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    InputEvent event;
    bool running = true;

    while(running) {
        if(furi_message_queue_get(state->queue, &event, 100) == FuriStatusOk) {
            if(event.key == InputKeyBack && event.type == InputTypePress) {
                running = false;
            }
        }
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(state->queue);
    free(state);

    return 0;
}
