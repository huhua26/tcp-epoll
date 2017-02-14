#-------------------------------------------------
#
# Project created by QtCreator 2016-02-23T10:49:43
#
#-------------------------------------------------

QT       -= core gui

TARGET = Epoll-Udp
TEMPLATE = lib

DEFINES += EPOLLUDP_LIBRARY

SOURCES += \
    ms-epoll-udp.cpp \
    udp-ms-epoll.cpp \
    udp-link.cpp

HEADERS +=  include/clientlog-api.h \
    include/Guard.h \
    include/singletonHandler.h \
    include/ms-epoll-udp.h \
    include/udp-link.h \
    include/udp-ms-epoll.h

INCLUDEPATH += "$$_PRO_FILE_PWD_/./include/"

unix {
    target.path = /usr/lib
    INSTALLS += target

    LIBS += -L"$$_PRO_FILE_PWD_/lib/" -Wl,-rpath="$$_PRO_FILE_PWD_/lib/"
    LIBS += -lClientLogDll
}
