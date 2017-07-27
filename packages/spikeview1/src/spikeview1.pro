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

INCLUDEPATH += msv/plugins msv/views msv/controls
VPATH += msv/plugins msv/views msv/controls

HEADERS += \
    mvthreadmanager.h \
    clustermetricsview.h clustermetricsplugin.h \
    mvgridview.h mvgridviewpropertiesdialog.h \
    templatesview.h templatesviewpanel.h templatesplugin.h templatescontrol.h \
    histogramview.h \
    crosscorview.h crosscorplugin.h crosscorcontrol.h \
    mvexportcontrol/mvexportcontrol.h

SOURCES += \
    mvthreadmanager.cpp \
    clustermetricsview.cpp clustermetricsplugin.cpp \
    mvgridview.cpp mvgridviewpropertiesdialog.cpp \
    templatesview.cpp templatesviewpanel.cpp templatesplugin.cpp templatescontrol.cpp \
    histogramview.cpp \
    crosscorview.cpp crosscorplugin.cpp crosscorcontrol.cpp \
    mvexportcontrol/mvexportcontrol.cpp

FORMS += mvgridviewpropertiesdialog.ui

HEADERS += mvopenviewscontrol.h get_sort_indices.h
SOURCES += mvopenviewscontrol.cpp get_sort_indices.cpp

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
