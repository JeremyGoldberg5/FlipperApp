#include <furi.h>

extern int32_t cpp_ide_app_main(void* p);

const FlipperAppDescriptor flipper_app_descriptor = {
    .id = "cpp_ide",
    .name = "C++ IDE",
    .author = "Jeremy Goldberg",
    .description = "C++ IDE with interpreter and code snippets",
    .category = FLIPPER_APP_CATEGORY_TOOLS,
    .flags = FlipperAppFlagInsomniaBit,
    .entry_point = cpp_ide_app_main,
};
