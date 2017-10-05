#!/bin/bash

set -e

# Synthesize some raw data (create waveforms.mda, geom.csv and raw.mda)
mp-run-process --_force_run pyms.synthesize_random_waveforms --waveforms_out=waveforms.mda --geometry_out=geom.csv --upsamplefac=13
mp-run-process mountainsort.synthesize_timeseries --waveforms=waveforms.mda --timeseries_out=raw.mda --num_timepoints=1e7 --waveform_upsamplefac=13

# Preprocessing (create filt.mda and pre.mda)
mp-run-process pyms.bandpass_filter --timeseries=raw.mda --timeseries_out=filt.mda --samplerate=30000 --freq_min=300 --freq_max=6000 --freq_wid=1000
mp-run-process pyms.normalize_channels --timeseries=filt.mda --timeseries_out=pre.mda

# Sorting (create firings.mda)
mp-run-process mountainsort.mountainsort3 --timeseries=pre.mda --firings_out=firings.mda --detect_threshold=3 --detect_sign=1

# View the results (launch the viewer)
mountainview --raw=raw.mda --filt=filt.mda --pre=pre.mda --firings=firings.mda --samplerate=30000