QT += core
QT -= gui
CONFIG -= app_bundle #Please apple, don't make a bundle today :)

CONFIG += c++11

include(../../../mlcommon/mlcommon.pri)
include(../../../mlcommon/mda.pri)

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainsort2.mp
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
    mountainsort2_main.cpp \
    p_extract_clips.cpp \
    p_extract_neighborhood_timeseries.cpp \
    p_detect_events.cpp \
    p_sort_clips.cpp \
    p_consolidate_clusters.cpp \
    p_create_firings.cpp \
    p_combine_firings.cpp \
    p_fit_stage.cpp \
    p_bandpass_filter.cpp \
    p_whiten.cpp \
    p_extract_segment_timeseries.cpp \
    p_apply_timestamp_offset.cpp \
    p_link_segments.cpp \
    p_cluster_metrics.cpp \
    p_split_firings.cpp \
    p_concat_timeseries.cpp \
    p_concat_firings.cpp \
    p_compute_templates.cpp

HEADERS += \
    p_extract_clips.h \
    mountainsort2_main.h \
    p_extract_neighborhood_timeseries.h \
    p_detect_events.h \
    p_sort_clips.h \
    p_consolidate_clusters.h \
    p_create_firings.h \
    p_combine_firings.h \
    p_fit_stage.h \
    p_bandpass_filter.h \
    p_whiten.h \
    p_extract_segment_timeseries.h \
    p_apply_timestamp_offset.h \
    p_link_segments.h \
    p_cluster_metrics.h \
    p_split_firings.h \
    p_concat_timeseries.h \
    p_concat_firings.h \
    p_compute_templates.h

INCLUDEPATH += ../../../mountainsort/src/isosplit5
VPATH += ../../../mountainsort/src/isosplit5
HEADERS += isosplit5.h isocut5.h jisotonic5.h
SOURCES += isosplit5.cpp isocut5.cpp jisotonic5.cpp

INCLUDEPATH += ../../../mountainsort/src/utils
VPATH += ../../../mountainsort/src/utils
HEADERS += pca.h get_sort_indices.h
SOURCES += pca.cpp get_sort_indices.cpp
