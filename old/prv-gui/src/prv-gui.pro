QT += core gui network widgets
CONFIG -= app_bundle #Please apple, don't make a bundle
CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-reorder #qaccordion

include(../../mlcommon/mlnetwork.pri)
include(../../mlcommon/taskprogress.pri)
include(../../mvcommon/mvcommon.pri)
include(../../mlcommon/mlcommon.pri)

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = prv-gui
TEMPLATE = app

SOURCES += prv-guimain.cpp \
    prvguimainwindow.cpp \
    prvguicontrolpanel.cpp \
    prvguitreewidget.cpp \
    prvgui.cpp \
    prvguimaincontrolwidget.cpp \
    prvguiuploaddialog.cpp \
    prvupload.cpp \
    prvdownload.cpp \
    prvguidownloaddialog.cpp \
    prvguiitemdetailwidget.cpp \
    locatemanager.cpp \
    locatemanagerworker.cpp \
    jsoneditorwindow.cpp \
    jsonmerger.cpp

INCLUDEPATH += ../../mountainview/src/core
HEADERS += \
    prvguimainwindow.h \
    prvguicontrolpanel.h \
    prvguitreewidget.h \
    prvgui.h \
    prvguimaincontrolwidget.h \
    prvguiuploaddialog.h \
    prvupload.h \
    prvdownload.h \
    prvguidownloaddialog.h \
    prvguiitemdetailwidget.h \
    locatemanager.h \
    locatemanagerworker.h \
    jsoneditorwindow.h \
    jsonmerger.h


FORMS += \
    prvguiuploaddialog.ui prvguidownloaddialog.ui \
    jsoneditorwindow.ui
