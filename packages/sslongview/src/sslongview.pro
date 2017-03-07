QT += core gui network

QT += qml

CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-reorder #qaccordion

CONFIG -= app_bundle #Please apple, don't make a bundle today

include(../../../mlcommon/mlcommon.pri)
include(../../../mlcommon/mda.pri)
include(../../../mlcommon/taskprogress.pri)
include(../../../controlwidgets/prvmanagerdialog/prvmanagerdialog.pri)

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

INCLUDEPATH += ../../../mountainview/src/core
VPATH += ../../../mountainview/src/core
HEADERS += \
    closemehandler.h flowlayout.h imagesavedialog.h \
    mountainprocessrunner.h mvabstractcontextmenuhandler.h \
    mvabstractcontrol.h mvabstractview.h mvabstractviewfactory.h \
    mvcontrolpanel2.h mvmainwindow.h mvstatusbar.h \
    mvabstractcontext.h tabber.h tabberframe.h taskprogressview.h actionfactory.h mvabstractplugin.h

SOURCES += \
    closemehandler.cpp flowlayout.cpp imagesavedialog.cpp \
    mountainprocessrunner.cpp mvabstractcontextmenuhandler.cpp \
    mvabstractcontrol.cpp mvabstractview.cpp mvabstractviewfactory.cpp \
    mvcontrolpanel2.cpp mvmainwindow.cpp mvstatusbar.cpp \
    mvabstractcontext.cpp tabber.cpp tabberframe.cpp taskprogressview.cpp actionfactory.cpp mvabstractplugin.cpp

INCLUDEPATH += ../../../mountainview/src/misc
VPATH += ../../../mountainview/src/misc
HEADERS += mvmisc.h mvutils.h paintlayer.h paintlayerstack.h renderablewidget.h multiscaletimeseries.h
SOURCES += mvmisc.cpp mvutils.cpp paintlayer.cpp paintlayerstack.cpp renderablewidget.cpp multiscaletimeseries.cpp

INCLUDEPATH += ../../../mountainsort/src/utils
VPATH += ../../../mountainsort/src/utils
HEADERS += get_sort_indices.h msmisc.h
SOURCES += get_sort_indices.cpp msmisc.cpp

INCLUDEPATH += ../../../prv-gui/src
VPATH += ../../../prv-gui/src
HEADERS += prvgui.h
SOURCES += prvgui.cpp

INCLUDEPATH += .
VPATH += ../../../mountainview/src

INCLUDEPATH += ../../../mountainview/src/3rdparty/qaccordion/include
VPATH += ../../../mountainview/src/3rdparty/qaccordion/include
VPATH += ../../../mountainview/src/3rdparty/qaccordion/src
HEADERS += qAccordion/qaccordion.h qAccordion/contentpane.h qAccordion/clickableframe.h
SOURCES += qaccordion.cpp contentpane.cpp clickableframe.cpp

RESOURCES += ../../../mountainview/src/mountainview.qrc \
    ../../../mountainview/src/3rdparty/qaccordion/icons/qaccordionicons.qrc

#TODO: Do we need openmp for mountainview?
#OPENMP
!macx {
  QMAKE_LFLAGS += -fopenmp
  QMAKE_CXXFLAGS += -fopenmp
}
#-std=c++11   # AHB removed since not in GNU gcc 4.6.3

