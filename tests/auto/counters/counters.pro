#-------------------------------------------------
#
# Project created by QtCreator 2016-10-16T14:10:53
#
#-------------------------------------------------

QT       += testlib qml

QT       -= gui

TARGET = tst_counterstest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app

include(../../../mlcommon/mlcommon.pri)


SOURCES += ../../../mountainview/src/misc/jscounter.cpp
INCLUDEPATH += ../../../mountainview/src/misc/
SOURCES += tst_counterstest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
