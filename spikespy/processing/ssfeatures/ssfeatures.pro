#-------------------------------------------------
#
# Project created by QtCreator 2015-03-28T17:43:59
#
#-------------------------------------------------

QT       += core
QT       -= gui

OBJECTS_DIR = build
MOC_DIR = build
DESTDIR = ../../bin
TARGET = ssfeatures
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp ssfeatures.cpp pcasolver.cpp array2d.cpp

HEADERS += ssfeatures.h pcasolver.h array2d.h

INCLUDEPATH += ../../../mountainsort/src/mda
HEADERS += ../../../mountainsort/src/mda/mdaio.h
SOURCES += ../../../mountainsort/src/mda/mdaio.cpp
SOURCES += ../../../mountainsort/src/mda/usagetracking.cpp

INCLUDEPATH += ../common
HEADERS += ../common/get_command_line_params.h
SOURCES += ../common/get_command_line_params.cpp