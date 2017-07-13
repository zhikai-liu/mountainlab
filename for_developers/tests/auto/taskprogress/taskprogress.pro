QT       += testlib

QT       -= gui

DESTDIR = ..
TARGET = tst_taskprogresstest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_taskprogresstest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
include(../setupdirs.pri)
include($${MOUNTAINLAB_CPP}/mlcommon/mlcommon.pri)
