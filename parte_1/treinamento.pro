TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    ../utils/connection_item.c \
    ../utils/connection_manager.c \
    ../utils/http_utils.c

DISTFILES +=

HEADERS += \
    ../utils/connection_item.h \
    ../utils/connection_manager,h \
    ../utils/http_utils.h