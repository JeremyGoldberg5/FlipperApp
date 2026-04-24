cmake_minimum_required(VERSION 3.22)

project(cpp_ide_app C)

add_library(cpp_ide_app OBJECT
    application.c
    interpreter/value.c
    interpreter/lexer.c
    interpreter/parser.c
    interpreter/executor.c
    editor/editor.c
    editor/snippets.c
)

target_include_directories(cpp_ide_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
