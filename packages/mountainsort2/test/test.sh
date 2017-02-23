#!/bin/bash

# You need to put a dataset called tetrode.mda in place

mp-run-process mountainsort.basic_sort --timeseries=tetrode.mda --firings_out=firings.mda --samplerate=30000 --_force_run --freq_min=300 --freq_max=6000 --freq_wid=1000 --whiten=1 --filt_out=filt.mda --pre_out=pre.mda --fit_stage=1 --consolidate_clusters=1
