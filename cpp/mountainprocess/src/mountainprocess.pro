QT = core qml

CONFIG += c++11

CONFIG += mlcommon

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainprocess
TEMPLATE = app

DEFINES += QT_MESSAGELOGCONTEXT

INCLUDEPATH += utils core mda unit_tests 3rdparty

HEADERS += \
    mpdaemon.h \
    mpdaemoninterface.h \
    localserver.h \
    handle_request.h \
    mllogmaster.h \
    mpdaemonmonitorinterface.h
SOURCES += \
    mpdaemon.cpp \
    mpdaemoninterface.cpp \
    localserver.cpp \
    handle_request.cpp \
    mllogmaster.cpp \
    mpdaemonmonitorinterface.cpp

HEADERS += \
    processmanager.h \
    scriptcontroller2.h \
    unit_tests/unit_tests.h

SOURCES += \
    processmanager.cpp \
    scriptcontroller2.cpp \
    unit_tests/unit_tests.cpp

#tests
test {
    QT += testlib
    CONFIG += testcase
    TARGET = mountainsort_test
    DEPENDPATH += unit_tests
    SOURCES += unit_tests/testMda.cpp	\
	unit_tests/testMain.cpp	\
	unit_tests/testMdaIO.cpp
    HEADERS += unit_tests/testMda.h \
	unit_tests/testMdaIO.h
} else {
    SOURCES += mountainprocessmain.cpp
}

include(../../installbin.pri)
