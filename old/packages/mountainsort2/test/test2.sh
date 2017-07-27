#!/bin/bash

OPTS="$1 --timeseries=$2"
OPTS="$OPTS --firings_out=firings.mda --geom=geom.csv --adjacency_radius=1000 --samplerate=30000"
OPTS="$OPTS --_force_run --freq_min=300 --freq_max=6000 --freq_wid=1000"
OPTS="$OPTS --whiten=1 --filt_out=filt.mda --pre_out=pre.mda"
OPTS="$OPTS --fit_stage=1 --consolidate_clusters=1 --segment_duration_sec=$3 --shift_duration_sec=$4"
OPTS="$OPTS --_preserve_tempdir --cluster_metrics_out=cluster_metrics.json"
OPTS="$OPTS --num_time_segment_threads=40"
mp-run-process $OPTS
