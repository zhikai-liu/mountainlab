TEMPLATE = subdirs

# usage:
# qmake
# qmake "COMPONENTS = mproc"

isEmpty(COMPONENTS) {
    COMPONENTS = mda mdaconvert mountainprocess mproc mountainsort prv mountainsort3 mountainsort2 mvcommon mountainview mountainsortalg
}

isEmpty(GUI) {
    GUI = on
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
SUBDIRS += $$ifcomponent(mproc,cpp/mproc/src/mproc.pro)
SUBDIRS += $$ifcomponent(mountainsort,cpp/mountainsort/src/mountainsort.pro)
SUBDIRS += $$ifcomponent(prv,cpp/prv/src/prv.pro)
SUBDIRS += $$ifcomponent(mountainsort2,packages/mountainsort2/src/mountainsort2.pro)
SUBDIRS += $$ifcomponent(mountainsort3,packages/mountainsort3/src/mountainsort3.pro)
SUBDIRS += $$ifcomponent(mountainsortalg,packages/mountainsortalg/mountainsortalg.pro)

equals(GUI,"on") {
  SUBDIRS += $$ifcomponent(mvcommon,cpp/mvcommon/src/mvcommon.pro)
  SUBDIRS += $$ifcomponent(mountainview,packages/mountainview/src/mountainview.pro)
}

DISTFILES += features/*
DISTFILES += debian/*

deb.target = deb
deb.commands = debuild $(DEBUILD_OPTS) -us -uc

QMAKE_EXTRA_TARGETS += deb
