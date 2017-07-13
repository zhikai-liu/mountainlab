#-------------------------------------------------
#
# Project created by QtCreator 2016-10-14T00:32:09
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

DESTDIR = ..
TARGET = tst_mlprivatetest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
include(../setupdirs.pri)
include($${MOUNTAINLAB_CPP}/mlcommon/mlcommon.pri)

SOURCES += tst_mlprivatetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
