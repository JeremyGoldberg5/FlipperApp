add_library(cpp_ide_app OBJECT
    application.c
    interpreter/value.c
    interpreter/lexer.c
    interpreter/parser.c
    interpreter/executor.c
    editor/editor.c
    editor/snippets.c
)

target_include_directories(cpp_ide_app PRIVATE .)
