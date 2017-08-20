QT = core network
CONFIG += c++11

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = prv
TEMPLATE = app

SOURCES += prvmain.cpp \
    prvfile.cpp

CONFIG += mlcommon taskprogress mlnetwork

