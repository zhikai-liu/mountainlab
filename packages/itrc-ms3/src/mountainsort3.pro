QT += core
QT -= gui
QT += qml

CONFIG += c++11

CONFIG += mlcommon fftw3

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainsort3.mp
TEMPLATE = app

#FFTW
USE_FFTW3=$$(USE_FFTW3)
CONFIG("no_fftw3") {
    warning(Not using FFTW3)
}
else {
    SOURCES += p_spikeview_templates.cpp
    HEADERS += p_spikeview_templates.h
}

#-std=c++11   # AHB removed since not in GNU gcc 4.6.3

SOURCES += \
    mountainsort3_main.cpp \
    #p_multineighborhood_sort.cpp \
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
    #p_preprocess.cpp \
    p_run_metrics_script.cpp \
    p_spikeview_metrics.cpp \
    p_synthesize_timeseries.cpp \
    p_combine_firing_segments.cpp \
    p_extract_firings.cpp \
    p_concat_timeseries.cpp \
    p_banjoview_cross_correlograms.cpp

INCLUDEPATH += ../../mountainsort2/src
VPATH += ../../mountainsort2/src
HEADERS += kdtree.h \
    p_concat_timeseries.h \
    p_banjoview_cross_correlograms.h
SOURCES += kdtree.cpp

HEADERS += \
    mountainsort3_main.h \
    #p_multineighborhood_sort.h \
    neighborhoodsorter.h \
    detect_events.h \
    sort_clips.h \
    consolidate_clusters.h \
    merge_across_channels.h \
    fit_stage.h \
    reorder_labels.h \
    p_mountainsort3.h \
    globaltemplatecomputer.h fitstagecomputer.h \
    #p_preprocess.h \
    p_run_metrics_script.h \
    p_spikeview_metrics.h \
    p_synthesize_timeseries.h \
    p_combine_firing_segments.h \
    p_extract_firings.h

INCLUDEPATH += ../../../cpp/mountainsort/src/isosplit5
VPATH += ../../../cpp/mountainsort/src/isosplit5
HEADERS += isosplit5.h isocut5.h jisotonic5.h
SOURCES += isosplit5.cpp isocut5.cpp jisotonic5.cpp

INCLUDEPATH += ../../../cpp/mountainsort/src/utils
VPATH += ../../../cpp/mountainsort/src/utils
HEADERS += pca.h compute_templates_0.h
SOURCES += pca.cpp compute_templates_0.cpp
