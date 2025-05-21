
TEMPLATE = lib

TARGET = ptcommlib

QT       -= gui
QT       += core xml network


QMAKE_CXXFLAGS += -std=c++11

DEFINES += ZCOMMONLIB
TOPDIR = $$PWD/../../public/
include($$TOPDIR/app.pri)


INCLUDEPATH += .\
              msg\
              ptdriver



SOURCES += \
    msg/DeviceMng.cpp \
    msg/MsgMng.cpp \
    msg/app.cpp \
    msg/driver.cpp \
    msg/msg.cpp \
    msg/shm.cpp \
    ptdriver/ptdrivercommon.cpp


HEADERS += \
    msg/DeviceMng.h \
    msg/MsgMng.h \
    msg/app.h \
    msg/define.h \
    msg/driver.h \
    msg/msg.h \
    msg/msgtype.h \
    msg/shm.h \
    ptdriver/ptdrivercommon.h



# Default rules for deployment.
DEPENDPATH += $${QBUILD}
