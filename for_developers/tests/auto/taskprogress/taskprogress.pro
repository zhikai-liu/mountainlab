QT       += testlib

QT       -= gui

TARGET = tst_taskprogresstest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_taskprogresstest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
include(../../../mlcommon/mlcommon.pri)
