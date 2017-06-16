QT += core
QT -= gui
CONFIG -= app_bundle #Please apple, don't make a bundle today :)

CONFIG += c++11

include(../../../mlcommon/mlcommon.pri)
include(../../../mlcommon/mda.pri)

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainsort3.mp
TEMPLATE = app

#FFTW
LIBS += -fopenmp -lfftw3 -lfftw3_threads

#OPENMP
!macx {
  QMAKE_LFLAGS += -fopenmp
  QMAKE_CXXFLAGS += -fopenmp
}
#-std=c++11   # AHB removed since not in GNU gcc 4.6.3

SOURCES += \
    mountainsort3_main.cpp \
    p_multineighborhood_sort.cpp \
    neighborhoodsorter.cpp \
    detect_events.cpp \
    dimension_reduce_clips.h \
    dimension_reduce_clips.cpp \
    sort_clips.cpp \
    consolidate_clusters.cpp \
    merge_across_channels.cpp \
    fit_stage.cpp \
    reorder_labels.cpp

HEADERS += \
    mountainsort3_main.h \
    p_multineighborhood_sort.h \
    neighborhoodsorter.h \
    detect_events.h \
    sort_clips.h \
    consolidate_clusters.h \
    merge_across_channels.h \
    fit_stage.h \
    reorder_labels.h

INCLUDEPATH += ../../../mountainsort/src/isosplit5
VPATH += ../../../mountainsort/src/isosplit5
HEADERS += isosplit5.h isocut5.h jisotonic5.h
SOURCES += isosplit5.cpp isocut5.cpp jisotonic5.cpp

INCLUDEPATH += ../../../mountainsort/src/utils
VPATH += ../../../mountainsort/src/utils
HEADERS += pca.h get_sort_indices.h compute_templates_0.h
SOURCES += pca.cpp get_sort_indices.cpp compute_templates_0.cpp
