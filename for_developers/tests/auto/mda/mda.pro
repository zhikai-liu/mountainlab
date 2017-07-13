QT       += testlib

QT       -= gui

DESTDIR = ..
TARGET = tst_mdatest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app
include(../setupdirs.pri)
include($${MOUNTAINLAB_CPP}/mlcommon/mlcommon.pri)
include($${MOUNTAINLAB_CPP}/mvcommon/mvcommon.pri)

SOURCES += tst_mdatest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
