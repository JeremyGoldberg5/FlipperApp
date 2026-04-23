# C++ IDE Application Makefile for Flipper Zero

APP_ID = cpp_ide
APP_CATEGORY = Tools
APP_ENTRY_POINT = cpp_ide_app_main
FAP_DESCRIPTION = C++ IDE with code editor and interpreter
FAP_AUTHOR = Jeremy Goldberg
FAP_VERSION = 1.0
FAP_ICON = icon.png

SOURCES = \
    application.c \
    interpreter/value.c \
    interpreter/lexer.c \
    interpreter/parser.c \
    interpreter/executor.c \
    editor/editor.c \
    editor/snippets.c \
    manifest.c

INCLUDES = \
    $(APP_DIR)

CFLAGS += -Wall -Wextra -O2

.PHONY: all clean

all: $(APP_ID).fap

clean:
	rm -f *.o *.fap *.elf
