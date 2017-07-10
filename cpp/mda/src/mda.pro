QT += core
QT -= gui

CONFIG += c++11

CONFIG -= app_bundle #Please apple, don't make a bundle

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mda
TEMPLATE = app

HEADERS +=

SOURCES += mdamain.cpp

include(../../mlcommon/mlcommon.pri)
include(../../mlcommon/mda.pri)
