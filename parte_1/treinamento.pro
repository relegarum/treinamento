TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    ../utils/http_utils.c

DISTFILES +=

HEADERS += \
    socket.h \
    ../utils/http_utils.h
