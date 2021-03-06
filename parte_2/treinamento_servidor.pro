CONFIG   += console
CONFIG   -= app_bundle

CONFIG -= qt

QMAKE_CFLAGS += -std=c++0x -pthread
LIBS += -pthread

TEMPLATE = app
SOURCES += main.c \
    ../utils/http_utils.c \
    ../utils/connection_item.c \
    ../utils/connection_manager.c \
    ../utils/test_suit.c \
    ../utils/file_utils.c \
    ../utils/thread.c \
    ../utils/request_list.c \
    ../utils/request_manager.c

HEADERS += \
    ../utils/http_utils.h \
    ../utils/connection_item.h \
    ../utils/connection_manager.h \
    ../utils/test_suit.h \
    ../utils/file_utils.h \
    ../utils/thread.h \
    ../utils/request_list.h \
    ../utils/request_manager.h
