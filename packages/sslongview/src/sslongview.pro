QT += core gui network

QT += qml

CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-reorder #qaccordion

CONFIG -= app_bundle #Please apple, don't make a bundle today

include(../../../mlcommon/mlcommon.pri)
include(../../../mlcommon/mda.pri)
include(../../../mlcommon/taskprogress.pri)
include(../../../mvcommon/mvcommon.pri)

QT += widgets
QT += concurrent

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = sslongview
TEMPLATE = app

INCLUDEPATH += plugins views
VPATH += plugins views

SOURCES += sslongviewmain.cpp

HEADERS += sslvcontext.h
SOURCES += sslvcontext.cpp

INCLUDEPATH += controlwidgets
VPATH += controlwidgets
HEADERS += sslvopenviewscontrol.h sslvprefscontrol.h
SOURCES += sslvopenviewscontrol.cpp sslvprefscontrol.cpp

HEADERS += sslvclustermetricsplugin.h sslvclustermetricsview.h
SOURCES += sslvclustermetricsplugin.cpp sslvclustermetricsview.cpp

HEADERS += sslvtimeseriesviewbase.h sslvtimespanview.h sslvtimespanplugin.h
SOURCES += sslvtimeseriesviewbase.cpp sslvtimespanview.cpp sslvtimespanplugin.cpp

HEADERS += clusterdetailplugin.h clusterdetailview.h clusterdetailviewpropertiesdialog.h
SOURCES += clusterdetailplugin.cpp clusterdetailview.cpp clusterdetailviewpropertiesdialog.cpp
FORMS += clusterdetailviewpropertiesdialog.ui

HEADERS += correlogramplugin.h mvhistogramgrid.h mvgridview.h histogramview.h mvgridviewpropertiesdialog.h correlogramwidget.h
SOURCES += correlogramplugin.cpp mvhistogramgrid.cpp mvgridview.cpp histogramview.cpp mvgridviewpropertiesdialog.cpp correlogramwidget.cpp
FORMS += mvgridviewpropertiesdialog.ui

HEADERS += mvfiringeventview2.h mvclusterlegend.h firingeventplugin.h
SOURCES += mvfiringeventview2.cpp mvclusterlegend.cpp firingeventplugin.cpp

INCLUDEPATH += ../../../mountainsort/src/utils
VPATH += ../../../mountainsort/src/utils
HEADERS += get_sort_indices.h msmisc.h
SOURCES += get_sort_indices.cpp msmisc.cpp

INCLUDEPATH += ../../../prv-gui/src
VPATH += ../../../prv-gui/src
HEADERS += prvgui.h
SOURCES += prvgui.cpp

INCLUDEPATH += ../../../mountainview/src/multiscaletimeseries
VPATH += ../../../mountainview/src/multiscaletimeseries
HEADERS += multiscaletimeseries.cpp
SOURCES += multiscaletimeseries.cpp


#TODO: Do we need openmp for mountainview?
#OPENMP
!macx {
  QMAKE_LFLAGS += -fopenmp
  QMAKE_CXXFLAGS += -fopenmp
}
#-std=c++11   # AHB removed since not in GNU gcc 4.6.3

