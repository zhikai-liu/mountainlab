#!/bin/bash

# You need to put a dataset called tetrode.mda in place
#mp-run-process bandpass_filter_aa --timeseries=tetrode.mda --timeseries_out=filt.mda --samplerate=30000 --freq_min=600 --freq_max=3000 --_request_num_threads=4

mountainprocess run-process bandpass_filter --timeseries=example1.mda --timeseries_out=filt.mda --samplerate=30000 --freq_min=300 --freq_max=6000
mountainprocess run-process whiten --timeseries=filt.mda --timeseries_out=pre.mda 
mp-run-process mountainsorter_v4 --timeseries=pre.mda --geom=geom.csv --firings_out=firings_out.mda --samplerate=30000 --adjacency_radius=1000 --_preserve_tempdir --_force_run
