QT += core gui network

QT += qml

CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-reorder #qaccordion

CONFIG -= app_bundle #Please apple, don't make a bundle today

include(../../../cpp/mvcommon/mvcommon.pri)
include(../../../cpp/mlcommon/mlcommon.pri)
include(../../../cpp/mlcommon/mda.pri)
include(../../../cpp/mlcommon/taskprogress.pri)

QT += widgets
QT += concurrent

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = spikeview1
TEMPLATE = app

RESOURCES += spikeview1.qrc

INCLUDEPATH += msv/plugins msv/views
VPATH += msv/plugins msv/views

HEADERS += \
    clustermetricsview.h \
    clustermetricsplugin.h

SOURCES += \
    clustermetricsview.cpp \
    clusterpairmetricsview.cpp

FORMS +=

SOURCES += spikeview1main.cpp

HEADERS += svcontext.h
SOURCES += svcontext.cpp

#TODO: Do we need openmp?
#OPENMP
!macx {
  QMAKE_LFLAGS += -fopenmp
  QMAKE_CXXFLAGS += -fopenmp
}
#-std=c++11   # AHB removed since not in GNU gcc 4.6.3
