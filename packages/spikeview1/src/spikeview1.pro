QT += core gui network qml

CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-reorder #qaccordion

#TODO: Do we need openmp?
CONFIG += mlcommon mvcommon openmp taskprogress

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
    mvexportcontrol/mvexportcontrol.h \
    spikeviewmetricscomputer.h \
    mvexportcontrol/exporttomountainviewdlg.h

SOURCES += \
    mvthreadmanager.cpp \
    clustermetricsview.cpp clustermetricsplugin.cpp \
    mvgridview.cpp mvgridviewpropertiesdialog.cpp \
    templatesview.cpp templatesviewpanel.cpp templatesplugin.cpp templatescontrol.cpp \
    histogramview.cpp \
    crosscorview.cpp crosscorplugin.cpp crosscorcontrol.cpp \
    mvexportcontrol/mvexportcontrol.cpp \
    spikeviewmetricscomputer.cpp \
    mvexportcontrol/exporttomountainviewdlg.cpp

FORMS += mvgridviewpropertiesdialog.ui \
    mvexportcontrol/exporttomountainviewdlg.ui

HEADERS += mvopenviewscontrol.h
SOURCES += mvopenviewscontrol.cpp

SOURCES += spikeview1main.cpp

HEADERS += svcontext.h
SOURCES += svcontext.cpp

#-std=c++11   # AHB removed since not in GNU gcc 4.6.3
