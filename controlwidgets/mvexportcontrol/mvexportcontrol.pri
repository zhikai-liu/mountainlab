INCLUDEPATH += $$PWD/include
LIB0 = $$PWD/lib/libprvmanagerdialog.a
LIBS += -L$$PWD/lib $$LIB0
unix:PRE_TARGETDEPS += $$LIB0

