TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    comm-func.cpp \
    md5.cpp \
    task-thread.cpp \
    timer-queue.cpp \
    test-client-handle.cpp \
    test-server.cpp \
    ../../tcp-link.cpp \
    ../../ms-epoll.cpp \
    ../../ms-epoll-tcp.cpp

HEADERS += \
    net-handler.h \
    comm_typedef.h \
    comm-func.h \
    link-hdr.h \
    database-typedef.h \
    link-mgr.h \
    md5.h \
    ms_endian.h \
    net-memory-io-base.h \
    net-packet.h \
    task-queue.h \
    task-thread.h \
    thread-queue.h \
    timer-queue.h \
    test-client-handle.h \
    test-server.h \
    ../../include/clientlog-api.h \
    ../../include/Guard.h \
    ../../include/ms-epoll-tcp.h \
    ../../include/ms-epoll.h \
    ../../include/singletonHandler.h \
    ../../include/tcp-link.h \
    include/comm_typedef.h \
    include/comm-func.h \
    include/database-typedef.h \
    include/link-hdr.h \
    include/link-mgr.h \
    include/md5.h \
    include/ms_endian.h \
    include/net-handler.h \
    include/net-memory-io-base.h \
    include/net-packet.h \
    include/task-queue.h \
    include/task-thread.h \
    include/test-client-handle.h \
    include/test-server.h \
    include/thread-queue.h \
    include/timer-queue.h

INCLUDEPATH += "$$_PRO_FILE_PWD_/./include/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/./../../include/"
unix {
    target.path = /usr/lib
    INSTALLS += target

    LIBS += -L"$$_PRO_FILE_PWD_/../../lib/" -Wl,-rpath="$$_PRO_FILE_PWD_/../../lib/"
    LIBS += -lClientLogDll
}


