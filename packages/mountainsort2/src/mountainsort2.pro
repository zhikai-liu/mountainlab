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
    p_extract_neighborhood_timeseries.cpp

HEADERS += \
    p_extract_clips.h \
    mountainsort2_main.h \
    p_extract_neighborhood_timeseries.h

