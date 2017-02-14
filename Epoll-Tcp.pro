#-------------------------------------------------
#
# Project created by QtCreator 2016-02-23T10:49:43
#
#-------------------------------------------------

QT       -= core gui

TARGET = Epoll-Tcp
TEMPLATE = lib

DEFINES += EPOLLTCP_LIBRARY

SOURCES += \
    tcp-link.cpp \
    ms-epoll.cpp \
    ms-epoll-tcp.cpp

HEADERS += \
    include/Guard.h \
    include/singletonHandler.h \
    include/tcp-link.h \
    include/ms-epoll.h \
    include/ms-epoll-tcp.h

INCLUDEPATH += "$$_PRO_FILE_PWD_/./include/"

unix {
    target.path = /usr/lib
    INSTALLS += target

#    LIBS += -L"$$_PRO_FILE_PWD_/lib/" -Wl,-rpath="$$_PRO_FILE_PWD_/lib/"
#    LIBS += -lClientLogDll
}
