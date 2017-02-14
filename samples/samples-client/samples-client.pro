TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    comm-func.cpp \
    test-client.cpp \
    md5.cpp \
    timer-queue.cpp \
    task-thread.cpp \
    ../../ms-epoll-tcp.cpp \
    ../../ms-epoll.cpp \
    ../../tcp-link.cpp


HEADERS += \
    net-handler.h \
    link-hdr.h \
    link-mgr.h \
    comm-func.h \
    comm_typedef.h \
    database-typedef.h \
    ms_endian.h \
    net-memory-io-base.h \
    net-packet.h \
    test-client.h \
    md5.h \
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
    include/test-client.h \
    include/thread-queue.h \
    include/timer-queue.h \
    ../../include/clientlog-api.h \
    ../../include/Guard.h \
    ../../include/ms-epoll-tcp.h \
    ../../include/ms-epoll.h \
    ../../include/singletonHandler.h \
    ../../include/tcp-link.h

INCLUDEPATH += "$$_PRO_FILE_PWD_/./include/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/./../../include/"

unix {
    target.path = /usr/lib
    INSTALLS += target

    LIBS += -L"$$_PRO_FILE_PWD_/../../lib/" -Wl,-rpath="$$_PRO_FILE_PWD_/../../lib/"
    LIBS += -lClientLogDll
}
