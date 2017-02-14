#-------------------------------------------------
#
# Project created by QtCreator 2016-03-23T11:56:05
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = udp-samples-client
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    comm-func.cpp \
    md5.cpp \
    task-thread.cpp \
    timer-queue.cpp \
    ../../udp-ms-epoll.cpp \
    ../../udp-link.cpp \
    ../../ms-epoll-udp.cpp \
    udp-client-test.cpp

HEADERS += \
    include/comm_typedef.h \
    include/comm-func.h \
    include/database-typedef.h \
    include/md5.h \
    include/ms_endian.h \
    include/net-memory-io-base.h \
    include/net-packet.h \
    include/task-queue.h \
    include/task-thread.h \
    include/thread-queue.h \
    include/timer-queue.h \
    ../../include/clientlog-api.h \
    ../../include/Guard.h \
    ../../include/udp-ms-epoll.h \
    ../../include/udp-link.h \
    ../../include/ms-epoll-udp.h \
    ../../include/singletonHandler.h \
    include/udp-nethandler.h \
    include/udp-client-test.h

INCLUDEPATH += "$$_PRO_FILE_PWD_/./include/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/./../../include/"

unix {
    target.path = /usr/lib
    INSTALLS += target

    LIBS += -L"$$_PRO_FILE_PWD_/../../lib/" -Wl,-rpath="$$_PRO_FILE_PWD_/../../lib/"
    LIBS += -lClientLogDll
}
