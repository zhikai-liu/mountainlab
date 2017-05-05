QT = core sql

CONFIG += c++11

CONFIG -= app_bundle #Please apple, don't make a bundle

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mlcache
TEMPLATE = app

HEADERS +=

SOURCES += main.cpp

include(../../mlcommon/mlcommon.pri)
