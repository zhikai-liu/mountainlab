#!/bin/bash

# You need to put a dataset called tetrode.mda in place
#mp-run-process bandpass_filter_aa --timeseries=tetrode.mda --timeseries_out=filt.mda --samplerate=30000 --freq_min=600 --freq_max=3000 --_request_num_threads=4

mp-run-process mountainsorter_v4 --timeseries=tetrode.mda --geom=geom.csv --firings_out=firings_out.mda --samplerate=30000 --adjacency_radius=100 --_preserve_tempdir
