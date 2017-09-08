QT = core qml

CONFIG += c++11

CONFIG += mlcommon

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mproc
TEMPLATE = app

HEADERS += mprocmain.h \
    processormanager.h \
    processresourcemonitor.h
SOURCES += mprocmain.cpp \
    processormanager.cpp \
    processresourcemonitor.cpp

EXTRA_INSTALLS = # (clear it out) used by installbin.pri
EXTRA_INSTALLS += "$$PWD/../bin_extra/*"
include(../../installbin.pri)
