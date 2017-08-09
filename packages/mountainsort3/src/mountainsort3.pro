QT += core
QT -= gui
QT += qml
CONFIG -= app_bundle #Please apple, don't make a bundle today :)

CONFIG += c++11

include(../../../cpp/mlcommon/mlcommon.pri)
include(../../../cpp/mlcommon/mda.pri)

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
    reorder_labels.cpp \
    p_mountainsort3.cpp \
    globaltemplatecomputer.cpp fitstagecomputer.cpp \
    p_preprocess.cpp \
    p_run_metrics_script.cpp \
    p_spikeview_metrics.cpp \
    p_spikeview_templates.cpp \
    p_synthesize_timeseries.cpp \
    p_combine_firing_segments.cpp

HEADERS += \
    mountainsort3_main.h \
    p_multineighborhood_sort.h \
    neighborhoodsorter.h \
    detect_events.h \
    sort_clips.h \
    consolidate_clusters.h \
    merge_across_channels.h \
    fit_stage.h \
    reorder_labels.h \
    p_mountainsort3.h \
    globaltemplatecomputer.h fitstagecomputer.h \
    p_preprocess.h \
    p_run_metrics_script.h \
    p_spikeview_metrics.h \
    p_spikeview_templates.h \
    p_synthesize_timeseries.h \
    p_combine_firing_segments.h

INCLUDEPATH += ../../../cpp/mountainsort/src/isosplit5
VPATH += ../../../cpp/mountainsort/src/isosplit5
HEADERS += isosplit5.h isocut5.h jisotonic5.h
SOURCES += isosplit5.cpp isocut5.cpp jisotonic5.cpp

INCLUDEPATH += ../../../cpp/mountainsort/src/utils
VPATH += ../../../cpp/mountainsort/src/utils
HEADERS += pca.h get_sort_indices.h compute_templates_0.h
SOURCES += pca.cpp get_sort_indices.cpp compute_templates_0.cpp
