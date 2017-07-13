#-------------------------------------------------
#
# Project created by QtCreator 2016-10-15T20:36:43
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

DESTDIR = ..
TARGET = tst_componentmanagertest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app

include(../setupdirs.pri)
include($${MOUNTAINLAB_CPP}/mlcommon/mlcommon.pri)

SOURCES += tst_componentmanagertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
