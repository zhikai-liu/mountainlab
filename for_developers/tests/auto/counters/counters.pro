#-------------------------------------------------
#
# Project created by QtCreator 2016-10-16T14:10:53
#
#-------------------------------------------------

QT       += testlib qml

QT       -= gui

DESTDIR = ../
TARGET = tst_counterstest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app

include(../setupdirs.pri)
include($${MOUNTAINLAB_CPP}/mlcommon/mlcommon.pri)
include($${MOUNTAINLAB_CPP}/mvcommon/mvcommon.pri)


INCLUDEPATH += $${MOUNTAINLAB_CPP}/mountainview/src/misc/
SOURCES += tst_counterstest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
