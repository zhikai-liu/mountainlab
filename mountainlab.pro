TEMPLATE = subdirs

# usage:
# qmake
# qmake "COMPONENTS = mountainview"

#COMPONENTS = mdaconvert  mountainprocess mountainview prv prv-gui

isEmpty(COMPONENTS) {
    COMPONENTS = mda mdaconvert mlcache mountainprocess mountainsort mountainview mountaincompare prv prv-gui mountainsort2
}

CONFIG += ordered

SUBDIRS += mlconfig/src/mlconfig.pro
SUBDIRS += mlcommon/src/mlcommon.pro
SUBDIRS += mvcommon/src/mvcommon.pro

defineReplace(ifcomponent) {
  contains(COMPONENTS, $$1) {
    message(Enabling $$1)
    return($$2)
  }
  return("")
}

SUBDIRS += $$ifcomponent(mdaconvert,mdaconvert/src/mdaconvert.pro)
SUBDIRS += $$ifcomponent(mda,mda/src/mda.pro)
SUBDIRS += $$ifcomponent(mlcache,mlcache/src/mlcache.pro)
SUBDIRS += $$ifcomponent(mountainbrowser,mountainbrowser/src/mountainbrowser.pro)
SUBDIRS += $$ifcomponent(mountainprocess,mountainprocess/src/mountainprocess.pro)
SUBDIRS += $$ifcomponent(mountainsort,mountainsort/src/mountainsort.pro)
SUBDIRS += $$ifcomponent(mountainview,mountainview/src/mountainview.pro)
SUBDIRS += $$ifcomponent(mountaincompare,mountaincompare/src/mountaincompare.pro)
SUBDIRS += $$ifcomponent(prv,prv/src/prv.pro)
SUBDIRS += $$ifcomponent(prv-gui,prv-gui/src/prv-gui.pro)
SUBDIRS += $$ifcomponent(mountainview-eeg,packages/mountainlab-eeg/mountainview-eeg/src/mountainview-eeg.pro)
SUBDIRS += $$ifcomponent(mountainsort2,packages/mountainsort2/src/mountainsort2.pro)
SUBDIRS += $$ifcomponent(sslongview,packages/sslongview/src/sslongview.pro)

CONFIG(debug, debug|release) { SUBDIRS += tests }
