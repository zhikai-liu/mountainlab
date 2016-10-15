#-------------------------------------------------
#
# Project created by QtCreator 2016-10-13T15:51:25
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_objectregistrytest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app

include(../../../mlcommon/mlcommon.pri)

SOURCES += tst_objectregistrytest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
