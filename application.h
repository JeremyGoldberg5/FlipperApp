#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_input.h>
#include <gui/modules/menu.h>
#include <gui/modules/popup.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>
#include <notification/notification_messages.h>

#include "interpreter/executor.h"

#define CPP_IDE_APP_PATH "/ext/cpp_ide"
#define MAX_FILE_NAME 64
#define MAX_CODE_SIZE 8192
#define MAX_OUTPUT_SIZE 4096

typedef enum {
    CppIdeViewMenu,
    CppIdeViewEditor,
    CppIdeViewSnippets,
    CppIdeViewOutput,
} CppIdeView;

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Menu* menu;
    TextInput* text_input;
    Popup* popup;
    NotificationApp* notifications;
    Storage* storage;
    DialogsApp* dialogs;

    char current_file[MAX_FILE_NAME];
    char code_buffer[MAX_CODE_SIZE];
    char output_buffer[MAX_OUTPUT_SIZE];
    uint16_t code_length;
    uint16_t output_length;
    uint16_t cursor_pos;
    uint16_t scroll_offset;
} CppIdeApp;

CppIdeApp* cpp_ide_app_alloc(void);
void cpp_ide_app_free(CppIdeApp* app);
