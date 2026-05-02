#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>

typedef struct {
    Gui* gui;
} AppState;

static void draw_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontDefault);
    canvas_draw_str(canvas, 10, 32, "C++ IDE");
}

int32_t cpp_ide_app_main(void* p) {
    UNUSED(p);

    AppState app = {0};
    app.gui = furi_record_open(RECORD_GUI);

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    gui_add_view_port(app.gui, view_port, GuiLayerFullscreen);

    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(uint8_t));

    while (true) {
        uint8_t message;
        if (furi_message_queue_get(queue, &message, FuriWaitForever) == FuriStatusOk) {
            if (message == 0) break;
        }
    }

    gui_remove_view_port(app.gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(queue);
    furi_record_close(RECORD_GUI);

    return 0;
}
