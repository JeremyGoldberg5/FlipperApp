#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>

static void draw_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 10, 20, "C++ IDE");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 10, 35, "Press BACK to exit");
}

static void input_callback(InputEvent* event, void* ctx) {
    if (event->key == InputKeyBack && event->type == InputTypePress) {
        furi_hal_power_off();
    }
}

int32_t test_app_main(void* p) {
    UNUSED(p);

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, NULL);

    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    furi_thread_yield();

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}
