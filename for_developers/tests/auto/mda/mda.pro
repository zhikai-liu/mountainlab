QT       += testlib

QT       -= gui

TARGET = tst_mdatest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app

include(../../../mlcommon/mlcommon.pri)
include(../../../mvcommon/mvcommon.pri)

SOURCES += tst_mdatest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
