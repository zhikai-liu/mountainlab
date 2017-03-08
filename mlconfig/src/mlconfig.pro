QT += core
QT -= gui

CONFIG += c++11

CONFIG -= app_bundle #Please apple, don't make a bundle

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR= ../build
TARGET = mlconfig
TEMPLATE = app

HEADERS += \
    mlconfigpage.h \
    mlconfigquestion.h

SOURCES += mlconfigmain.cpp \
    mlconfigpage.cpp \
    mlconfigquestion.cpp


