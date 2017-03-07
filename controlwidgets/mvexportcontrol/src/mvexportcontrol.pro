QT = core network
QT += widgets

include(../../../mlcommon/mlcommon.pri)
include(../../../mlcommon/taskprogress.pri)

CONFIG += c++11
CONFIG -= app_bundle
CONFIG += staticlib

DESTDIR = ../lib
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = prvmanagerdialog
TEMPLATE = lib

INCLUDEPATH += ../include
VPATH += ../include
HEADERS += prvmanagerdialog.h resolveprvsdialog.h mvexportcontrol.h flowlayout.h
SOURCES += prvmanagerdialog.cpp resolveprvsdialog.cpp mvexportcontrol.cpp flowlayout.cpp
FORMS += prvmanagerdialog.ui resolveprvsdialog.ui

DISTFILES += \
    ../prvmanagerdialog.pri
