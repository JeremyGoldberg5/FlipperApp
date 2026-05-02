#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>

int32_t test_app_main(void* p) {
    UNUSED(p);

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();

    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Simple event loop
    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(uint8_t));
    while (true) {
        uint8_t message;
        if (furi_message_queue_get(queue, &message, 100) == FuriStatusOk) {
            if (message == 0) break;
        }
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(queue);
    furi_record_close(RECORD_GUI);

    return 0;
}
