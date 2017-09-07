TEMPLATE = subdirs

# usage:
# qmake
# qmake "COMPONENTS = mountainprocess"

#COMPONENTS = mdaconvert  mountainprocess prv

#!CONFIG("no_fftw3"):!packagesExist(fftw3) {
#  echo("FFTW3 does not seem to be installed on your system. Please install it.")
#  echo("On systems like Ubuntu you can run sudo apt install libfftw-dev.")
#  echo("Alternatively, you can disable fftw3 (which will disable some functionality),")
#  echo("you can make a .qmake.cache file in this directory with the following line:")
#  echo("CONFIG += no_fftw3.")
#  echo("You can also use .qmake.cache to provide other qmake settings that may allow you")
#  echo("to find fftw in a different location.")
#  echo("")
#  error("Aborting")
#}

isEmpty(COMPONENTS) {
    COMPONENTS = mda mdaconvert mountainprocess mountainsort prv mountainsort3 mountainsort2
}

CONFIG += ordered

defineReplace(ifcomponent) {
  contains(COMPONENTS, $$1) {
    message(Enabling $$1)
    return($$2)
  }
  return("")
}

SUBDIRS += cpp/mlcommon/src/mlcommon.pro
SUBDIRS += cpp/mlconfig/src/mlconfig.pro
SUBDIRS += $$ifcomponent(mdaconvert,cpp/mdaconvert/src/mdaconvert.pro)
SUBDIRS += $$ifcomponent(mda,cpp/mda/src/mda.pro)
SUBDIRS += $$ifcomponent(mountainprocess,cpp/mountainprocess/src/mountainprocess.pro)
SUBDIRS += $$ifcomponent(mountainsort,cpp/mountainsort/src/mountainsort.pro)
SUBDIRS += $$ifcomponent(prv,cpp/prv/src/prv.pro)
SUBDIRS += $$ifcomponent(mountainsort2,packages/mountainsort2/src/mountainsort2.pro)
SUBDIRS += $$ifcomponent(mountainsort3,packages/mountainsort3/src/mountainsort3.pro)

DISTFILES += features/*
DISTFILES += debian/*

deb.target = deb
deb.commands = debuild $(DEBUILD_OPTS) -us -uc

QMAKE_EXTRA_TARGETS += deb
