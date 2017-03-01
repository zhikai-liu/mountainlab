#!/bin/bash

PATH1="output/m2drift--ds1/session_1"
PATH2="output/m2drift--ds1/session_2"
PATH3="output/m2drift--ds1/session_3"

mp-run-process mountainsort.concat_firings \
	--timeseries_list=$PATH1/raw.mda.prv --timeseries_list=$PATH2/raw.mda.prv --timeseries_list=$PATH3/raw.mda.prv \
	--firings_list=$PATH1/firings.mda --firings_list=$PATH2/firings.mda --firings_list=$PATH3/firings.mda \
	--timeseries_out=ts_combined.mda \
	--firings_out=firings_combined.mda

